#pragma once

#include "routines/handler.hpp"
#include "utils/unit.hpp"

#include <yaclib/async/make.hpp>

namespace ds::echo {
class EchoHandler final : public maelstrom::HandlerBase<maelstrom::Unit> {
public:
  static constexpr std::string_view kType = "echo";

  static yaclib::Future<maelstrom::Response>
  handle([[maybe_unused]] maelstrom::Network::Session &&session,
         maelstrom::Request &&request) override {
    auto body = nlohmann::json({});
    body["echo"] = request.body["echo"];
    return yaclib::MakeFuture(std::move(request).toResponse(std::move(body)));
  }
};
} // namespace ds::echo