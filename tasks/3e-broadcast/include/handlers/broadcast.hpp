#pragma once

#include "routines/handler.hpp"

#include "state.hpp"

namespace ds::broadcast {
  class BroadcastHandler final : public maelstrom::HandlerBase<State> {
  public:
    static constexpr std::string_view type = "broadcast";

    yaclib::Future<maelstrom::Response>
    handle(maelstrom::Network::Session&& session,
           maelstrom::Request&& request) override;
  };

  class BroadcastBulkHandler final : public maelstrom::HandlerBase<State> {
  public:
    static constexpr std::string_view type = "broadcast_bulk";

    yaclib::Future<maelstrom::Response>
    handle(maelstrom::Network::Session&& session,
           maelstrom::Request&& request) override;
  };
}// namespace ds::broadcast