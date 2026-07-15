#pragma once

#include "routines/handler.hpp"
#include "utils/unit.hpp"

namespace ds::gcounter {
  class AddHandler final : public maelstrom::HandlerBase<maelstrom::Unit> {
  public:
    static constexpr std::string_view type = "add";

    yaclib::Future<maelstrom::Response>
    handle(maelstrom::Network::Session&& session,
           maelstrom::Request&& reqeust) override;
  };

  class ReadHandler final : public maelstrom::HandlerBase<maelstrom::Unit> {
  public:
    static constexpr std::string_view type = "read";

    yaclib::Future<maelstrom::Response>
    handle(maelstrom::Network::Session&& session,
           maelstrom::Request&& reqeust) override;
  };
}// namespace ds::gcounter