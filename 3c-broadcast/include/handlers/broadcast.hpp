#pragma once

#include <yaclib/async/make.hpp>

#include "routines/handler.hpp"
#include "network/messages.hpp"
#include "network/network.hpp"

#include "state.hpp"

namespace ds::broadcast {
  class BroadcastHandler final : public core::HandlerBase<State> {
  public:
    static constexpr std::string_view type = "broadcast";

    yaclib::Future<core::Response> handle(core::Network::Session&& session,
                                          core::Request&& request) override {
      auto num = request.body["message"].get<std::uint64_t>();

      {
        std::lock_guard guard{env_->state->mtx_};
        env_->state->nums.push_back(num);
      }

      doSpreading(session, request, num);

      return yaclib::MakeFuture(std::move(request).toResponse());
    }

  private:
    void doSpreading(core::Network::Session& session,
                     const core::Request& request, std::uint64_t num) {
      auto& node_id = env_->node_id;
      auto& topology = env_->state->topology.value();

      std::vector<std::string> visited;
      if (request.body.contains("visited")) {
        visited = request.body["visited"].get<std::vector<std::string>>();
      }

      visited.push_back(env_->node_id);

      auto spread_body = nlohmann::json({});
      spread_body["visited"] = visited;
      spread_body["message"] = num;
      for (auto& id: topology[node_id]) {
        if (id != node_id &&
            std::find(visited.begin(), visited.end(), id) == visited.end()) {
          session.sendAtLeastOnce<BroadcastHandler>(id, spread_body);
        }
      }
    }
  };
}// namespace ds::broadcast