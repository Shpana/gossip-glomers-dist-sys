#pragma once

#include "routines/handler.hpp"

#include "state.hpp"

namespace ds::broadcast {
  class TopologyHandler final : public core::HandlerBase<State> {
  public:
    static constexpr std::string_view type = "topology";

    yaclib::Future<core::Response> handle(core::Network::Session&& session,
                                          core::Request&& request) override;
  };
}// namespace ds::broadcast