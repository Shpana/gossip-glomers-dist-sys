#include "workers/distributor.hpp"

#include <yaclib/async/make.hpp>
#include <yaclib/async/when_all.hpp>

#include "network/messages.hpp"
#include "network/network.hpp"

#include "handlers/broadcast.hpp"

namespace ds::broadcast {
  yaclib::Future<>
  DistributorWorker::process(maelstrom::Network::Session&& session) {
    LOG_INFO() << "distributor called!\n";

    auto& info = state_->info;
    auto& storage = state_->storage;

    if (!info.ready.load()) {
      return yaclib::MakeFuture();
    }

    std::vector<std::string> all_node_ids;
    {
      std::lock_guard guard{info.mtx};
      all_node_ids = info.all_node_ids;
    }

    std::vector<yaclib::Future<>> futs;
    futs.reserve(all_node_ids.size());

    using namespace std::chrono_literals;
    constexpr maelstrom::Network::Clock::duration timeout{300ms};

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
                                yaclib::Result<maelstrom::Response> result) {
                  if (result) {
                    std::lock_guard guard{storage.mtx};
                    storage.maybe_unknown[node_id] = prev_end;
                  }
                }));
      }
    }

    if (futs.empty()) {
      return yaclib::MakeFuture();
    }

    return yaclib::WhenAll(futs.begin(), futs.end()).ThenInline([](auto&&) {});
  }
}// namespace ds::broadcast