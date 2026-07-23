#include "workers/distributor.hpp"

#include "handlers/broadcast.hpp"

#include <yaclib/async/make.hpp>
#include <yaclib/async/when_all.hpp>

#include <maelstrom/network/messages.hpp>
#include <maelstrom/network/network.hpp>

namespace tasks::broadcast::part5 {

yaclib::Future<>
DistributorWorker::Process(maelstrom::Network::Session session) {
  auto &info = GetState().info;
  auto &storage = GetState().storage;

  using namespace std::chrono_literals;
  constexpr maelstrom::Network::Clock::duration kTimeout{300ms};

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

  {
    std::lock_guard guard{storage.mtx};

    for (auto &node_id : all_node_ids) {
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
        session.Call<BroadcastBulkHandler>(node_id, std::move(body), kTimeout)
          .ThenInline([&storage, node_id, prev_end = storage.nums.size()](
                        yaclib::Result<maelstrom::Response> &&result) {
            if (result) {
              std::lock_guard guard{storage.mtx};
              storage.maybe_unknown[node_id] = prev_end;
            }
          }));
    }
  }

  if (futs.empty()) {
    return {};
  }

  return yaclib::WhenAll(futs.begin(), futs.end());
}

} // namespace tasks::broadcast::part5