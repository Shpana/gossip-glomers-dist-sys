#pragma once

#include "state.hpp"

#include <maelstrom/routines/handler.hpp>

namespace tasks::broadcast::part4 {

class BroadcastHandler final : public maelstrom::HandlerBase<State> {
public:
  static constexpr std::string_view kType = "broadcast";

  yaclib::Future<maelstrom::Response>
  Handle(maelstrom::Network::Session session,
         maelstrom::Request request) override;
};

class BroadcastBulkHandler final : public maelstrom::HandlerBase<State> {
public:
  static constexpr std::string_view kType = "broadcast_bulk";

  yaclib::Future<maelstrom::Response>
  Handle(maelstrom::Network::Session session,
         maelstrom::Request request) override;
};

} // namespace tasks::broadcast::part4