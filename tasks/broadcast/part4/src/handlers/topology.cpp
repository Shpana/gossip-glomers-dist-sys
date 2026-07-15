#include "handlers/topology.hpp"

#include <yaclib/async/make.hpp>

namespace ds::broadcast {
  yaclib::Future<maelstrom::Response> TopologyHandler::handle(
      [[maybe_unused]] maelstrom::Network::Session&& session,
      maelstrom::Request&& request) {
    auto& info = env_->state->info;
    auto& storage = env_->state->storage;

    auto topology =
        request.body["topology"]
            .get<std::unordered_map<std::string, std::vector<std::string>>>();

    {
      std::lock_guard guard{info.mtx};
      info.topology = std::move(topology);

      for (auto& [node_id, neighbours]: info.topology) {
        info.all_node_ids.push_back(node_id);
      }
    }

    info.ready.store(true);

    return yaclib::MakeFuture(std::move(request).toResponse());
  }
}// namespace ds::broadcast