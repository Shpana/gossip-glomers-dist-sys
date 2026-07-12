#pragma once

#include "routines/handler.hpp"

#include "state.hpp"

namespace ds::broadcast {
  class BroadcastHandler final : public core::HandlerBase<State> {
  public:
    static constexpr std::string_view type = "broadcast";

    yaclib::Future<core::Response> handle(core::Network::Session&& session,
                                          core::Request&& request) override;
  };

  class BroadcastBulkHandler final : public core::HandlerBase<State> {
  public:
    static constexpr std::string_view type = "broadcast_bulk";

    yaclib::Future<core::Response> handle(core::Network::Session&& session,
                                          core::Request&& request) override;
  };
}// namespace ds::broadcast