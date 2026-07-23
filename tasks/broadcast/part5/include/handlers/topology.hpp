#pragma once

#include "state.hpp"

#include <maelstrom/routines/handler.hpp>

namespace tasks::broadcast::part5 {

class TopologyHandler final : public maelstrom::HandlerBase<State> {
public:
  static constexpr std::string_view kType = "topology";

  yaclib::Future<maelstrom::Response>
  Handle(maelstrom::Network::Session session,
         maelstrom::Request request) override;
};

} // namespace tasks::broadcast::part5