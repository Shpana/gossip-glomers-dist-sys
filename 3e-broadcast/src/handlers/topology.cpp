#include "handlers/topology.hpp"

#include <yaclib/async/make.hpp>

namespace ds::broadcast {
  yaclib::Future<core::Response>
  TopologyHandler::handle([[maybe_unused]] core::Network::Session&& session,
                          core::Request&& request) {
    auto& info = state_->info;
    auto& storage = state_->storage;

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