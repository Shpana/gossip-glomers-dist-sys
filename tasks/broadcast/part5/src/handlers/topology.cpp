#include "handlers/topology.hpp"

#include <yaclib/async/make.hpp>

namespace tasks::broadcast::part4 {

yaclib::Future<maelstrom::Response>
TopologyHandler::Handle(maelstrom::Network::Session session,
                        maelstrom::Request request) {
  auto &info = GetState().info;
  auto &storage = GetState().storage;

  auto topology =
    request.body["topology"]
      .get<std::unordered_map<std::string, std::vector<std::string>>>();

  {
    std::lock_guard guard{info.mtx};
    info.topology = std::move(topology);
  }

  return yaclib::MakeFuture(std::move(request).ToResponse());
}

} // namespace tasks::broadcast::part4