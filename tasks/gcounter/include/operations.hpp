#pragma once

#include "state.hpp"

#include <maelstrom/network/messages.hpp>
#include <maelstrom/network/network.hpp>
#include <maelstrom/routines/handler.hpp>

namespace tasks::gcounter {

class AddHandler final : public maelstrom::HandlerBase<State> {
public:
  static constexpr std::string_view kType = "add";

  yaclib::Future<maelstrom::Response>
  Handle(maelstrom::Network::Session session,
         maelstrom::Request request) override;
};

class ReadHandler final : public maelstrom::HandlerBase<State> {
public:
  static constexpr std::string_view kType = "read";

  yaclib::Future<maelstrom::Response>
  Handle(maelstrom::Network::Session session,
         maelstrom::Request request) override;

private:
  yaclib::Future<std::uint64_t> Read(maelstrom::Network::Session &session);
};

} // namespace tasks::gcounter