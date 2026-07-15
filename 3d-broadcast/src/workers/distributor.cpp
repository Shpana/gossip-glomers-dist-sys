#include "workers/distributor.hpp"

#include <yaclib/async/make.hpp>
#include <yaclib/async/when_all.hpp>

#include "network/messages.hpp"
#include "network/network.hpp"

#include "handlers/broadcast.hpp"
#include "utils/unit.hpp"

namespace ds::broadcast {
  yaclib::Future<core::Unit>
  DistributorWorker::process(core::Network::Session&& session) {
    auto& info = env_->state->info;
    auto& storage = env_->state->storage;

    if (!info.ready.load()) {
      return yaclib::MakeFuture(core::Unit{});
    }

    std::vector<std::string> all_node_ids;
    {
      std::lock_guard guard{info.mtx};
      all_node_ids = info.all_node_ids;
    }

    std::vector<yaclib::FutureOn<core::Unit>> futs;
    futs.reserve(all_node_ids.size());

    using namespace std::chrono_literals;
    constexpr core::Network::Clock::duration timeout{300ms};

    {
      std::lock_guard guard{storage.mtx};

      for (auto& node_id: all_node_ids) {
        if (!storage.maybe_unknown.contains(node_id)) [[unlikely]] {
          storage.maybe_unknown[node_id] = 0;
        }

        if (storage.maybe_unknown[node_id] == storage.nums.size()) {
          continue;
        }

        auto body = nlohmann::json({});
        body["messages"] = std::vector<std::uint64_t>{
            storage.nums.begin() + storage.maybe_unknown[node_id],
            storage.nums.end()};

        futs.push_back(
            session
                .call<BroadcastBulkHandler>(node_id, std::move(body), timeout)
                .ThenInline([&storage, node_id, prev_end = storage.nums.size()](
                                yaclib::Result<core::Response> result) {
                  if (result) {
                    std::lock_guard guard{storage.mtx};
                    storage.maybe_unknown[node_id] = prev_end;
                  }
                  return core::Unit{};
                }));
      }
    }

    if (futs.empty()) {
      return yaclib::MakeFuture(core::Unit{});
    }

    return yaclib::WhenAll(futs.begin(), futs.end())
        .ThenInline([](auto&& result) {
          return core::Unit{};
        });
  }
}// namespace ds::broadcast