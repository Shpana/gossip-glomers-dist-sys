#pragma once

#include "state.hpp"

#include <yaclib/async/make.hpp>

#include <maelstrom/network/messages.hpp>
#include <maelstrom/network/network.hpp>
#include <maelstrom/routines/handler.hpp>

namespace tasks::broadcast::part3 {

class BroadcastHandler final : public maelstrom::HandlerBase<State> {
public:
  static constexpr std::string_view kType = "broadcast";

  yaclib::Future<maelstrom::Response>
  Handle(maelstrom::Network::Session session,
         maelstrom::Request request) override {
    auto &state = GetState();

    auto num = request.body["message"].get<std::uint64_t>();

    {
      std::lock_guard guard{state.mtx};
      state.nums.push_back(num);
    }

    DoSpreading(session, request, num);

    return yaclib::MakeFuture(std::move(request).ToResponse());
  }

private:
  void DoSpreading(maelstrom::Network::Session &session,
                   const maelstrom::Request &request, std::uint64_t num) {
    auto &node_id = GetEnvironment().node_id;
    auto &topology = GetState().topology.value();

    std::vector<std::string> visited;
    if (request.body.contains("visited")) {
      visited = request.body["visited"].get<std::vector<std::string>>();
    }

    visited.push_back(GetEnvironment().node_id);

    auto spread_body = nlohmann::json({});
    spread_body["visited"] = visited;
    spread_body["message"] = num;
    for (auto &id : topology[node_id]) {
      if (id != node_id &&
          std::find(visited.begin(), visited.end(), id) == visited.end()) {
        session.SendAtLeastOnce<BroadcastHandler>(id, spread_body);
      }
    }
  }
};

class ReadHandler final : public maelstrom::HandlerBase<State> {
public:
  static constexpr std::string_view kType = "read";

  yaclib::Future<maelstrom::Response>
  Handle(maelstrom::Network::Session session,
         maelstrom::Request request) override {
    auto &state = GetState();

    auto body = nlohmann::json({});
    body["messages"] = state.nums;
    return yaclib::MakeFuture(std::move(request).ToResponse(std::move(body)));
  }
};

class TopologyHandler final : public maelstrom::HandlerBase<State> {
public:
  static constexpr std::string_view kType = "topology";

  yaclib::Future<maelstrom::Response>
  Handle(maelstrom::Network::Session session,
         maelstrom::Request request) override {
    auto &state = GetState();

    state.topology.emplace(
      request.body["topology"]
        .get<std::unordered_map<std::string, std::vector<std::string>>>());

    return yaclib::MakeFuture(std::move(request).ToResponse());
  }
};

} // namespace tasks::broadcast::part3