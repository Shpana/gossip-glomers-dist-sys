#pragma once

#include <yaclib/async/make.hpp>

#include "network/messages.hpp"
#include "network/network.hpp"
#include "routines/handler.hpp"

#include "state.hpp"

namespace ds::broadcast {
  class BroadcastHandler final : public maelstrom::HandlerBase<State> {
  public:
    static constexpr std::string_view type = "broadcast";

    yaclib::Future<maelstrom::Response>
    handle(maelstrom::Network::Session&& session,
           maelstrom::Request&& request) override {
      auto num = request.body["message"].get<std::uint64_t>();

      {
        std::lock_guard guard{env_->state->mtx_};
        env_->state->nums.push_back(num);
      }

      doSpreading(session, request, num);

      return yaclib::MakeFuture(std::move(request).toResponse());
    }

  private:
    void doSpreading(maelstrom::Network::Session& session,
                     const maelstrom::Request& request, std::uint64_t num) {
      auto& node_id = env_->node_id;
      auto& topology = env_->state->topology.value();

      std::vector<std::string> visited;
      if (request.body.contains("visited")) {
        visited = request.body["visited"].get<std::vector<std::string>>();
      }

      std::vector<std::string> next_visited;
      next_visited.reserve(visited.size() + topology[node_id].size() + 1);

      next_visited.push_back(env_->node_id);
      for (auto& id: topology[node_id]) {
        next_visited.push_back(id);
      }
      for (auto& id: visited) {
        next_visited.push_back(id);
      }

      auto spread_body = nlohmann::json({});
      spread_body["visited"] = std::move(next_visited);
      spread_body["message"] = num;
      for (auto& id: topology[node_id]) {
        if (id != node_id &&
            std::find(visited.begin(), visited.end(), id) == visited.end()) {
          session.send<BroadcastHandler>(id, spread_body);
        }
      }
    }
  };
}// namespace ds::broadcast