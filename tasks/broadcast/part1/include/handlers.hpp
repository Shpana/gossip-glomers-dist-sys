#pragma once

#include "state.hpp"

#include <yaclib/async/make.hpp>

#include <maelstrom/routines/handler.hpp>

namespace tasks::broadcast::part1 {

class BroadcastHandler final : public maelstrom::HandlerBase<State> {
public:
  static constexpr std::string_view kType = "broadcast";

  yaclib::Future<maelstrom::Response>
  Handle(maelstrom::Network::Session session,
         maelstrom::Request request) override {
    auto &state = GetState();

    auto num = request.body["message"].get<std::uint64_t>();
    state.nums.push_back(num);
    return yaclib::MakeFuture(std::move(request).ToResponse());
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

} // namespace tasks::broadcast::part1