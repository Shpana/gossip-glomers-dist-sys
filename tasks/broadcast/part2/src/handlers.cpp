#include "handlers.hpp"

namespace tasks::broadcast::part2 {

void BroadcastHandler::DoSpreading(maelstrom::Network::Session &session,
                                   const maelstrom::Request &request,
                                   std::uint64_t num) {
  auto &node_id = GetEnvironment().node_id;
  auto &topology = GetState().topology.value();

  std::vector<std::string> visited;
  if (request.body.contains("visited")) {
    visited = request.body["visited"].get<std::vector<std::string>>();
  }

  std::vector<std::string> next_visited;
  next_visited.reserve(visited.size() + topology[node_id].size() + 1);

  next_visited.push_back(node_id);
  for (auto &id : topology[node_id]) {
    next_visited.push_back(id);
  }
  for (auto &id : visited) {
    next_visited.push_back(id);
  }

  auto spread_body = nlohmann::json({});
  spread_body["visited"] = std::move(next_visited);
  spread_body["message"] = num;
  for (auto &id : topology[node_id]) {
    if (id != node_id &&
        std::find(visited.begin(), visited.end(), id) == visited.end()) {
      session.Send<BroadcastHandler>(id, spread_body);
    }
  }
}

} // namespace tasks::broadcast::part2