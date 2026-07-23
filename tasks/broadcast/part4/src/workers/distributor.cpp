#include "workers/distributor.hpp"

#include "handlers/broadcast.hpp"

#include <yaclib/async/make.hpp>
#include <yaclib/async/when_all.hpp>
#include <yaclib/coro/await.hpp>
#include <yaclib/coro/future.hpp>

#include <maelstrom/network/messages.hpp>
#include <maelstrom/network/network.hpp>
#include <maelstrom/utils/unit.hpp>

namespace tasks::broadcast::part4 {

void DistributorWorker::Start() {
  constexpr std::size_t kMaxPathLength = 3;

  auto nodes_count = GetEnvironment().available_node_ids.size();
  auto batch_size = (nodes_count + kMaxPathLength - 1) / kMaxPathLength;
  node_ids_for_notify_.reserve(batch_size);

  auto from = std::stoull(std::string{GetEnvironment().node_id.begin() + 1,
                                      GetEnvironment().node_id.end()});

  for (std::size_t i = 0; i < batch_size; ++i) {
    auto batch_index = from / batch_size;
    auto to = ((batch_index + 1) * batch_size + i) % nodes_count;
    node_ids_for_notify_.push_back("n" + std::to_string(to));
  }
}

yaclib::Future<>
DistributorWorker::Process(maelstrom::Network::Session session) {
  auto &info = GetState().info;
  auto &storage = GetState().storage;

  {
    std::lock_guard guard{storage.mtx};

    for (auto &node_id : node_ids_for_notify_) {
      if (storage.maybe_unknown[node_id] == storage.nums.size()) {
        // All known nums already was broadcasted to that node
        continue;
      }

      auto body = nlohmann::json({});
      body["messages"] = std::vector<std::uint64_t>{
        storage.nums.begin() + storage.maybe_unknown[node_id],
        storage.nums.end()};

      // Sending without waiting for response
      // We assume that broadcast handler is idemponental
      storage.maybe_unknown[node_id] = storage.nums.size();
      std::ignore =
        session.SendAtLeastOnce<BroadcastBulkHandler>(node_id, std::move(body));
    }
  }

  co_return {};
}

} // namespace tasks::broadcast::part4