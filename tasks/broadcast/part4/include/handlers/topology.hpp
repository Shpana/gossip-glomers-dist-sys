#pragma once

#include "routines/handler.hpp"

#include "state.hpp"

namespace ds::broadcast {
  class TopologyHandler final : public maelstrom::HandlerBase<State> {
  public:
    static constexpr std::string_view type = "topology";

    yaclib::Future<maelstrom::Response>
    handle(maelstrom::Network::Session&& session,
           maelstrom::Request&& request) override;
  };
}// namespace ds::broadcast