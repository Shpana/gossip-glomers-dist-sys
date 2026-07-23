#pragma once

#include <yaclib/async/make.hpp>

#include <maelstrom/routines/handler.hpp>
#include <maelstrom/utils/unit.hpp>

namespace tasks::echo {

class EchoHandler final : public maelstrom::HandlerBase<maelstrom::Unit> {
public:
  static constexpr std::string_view kType = "echo";

  yaclib::Future<maelstrom::Response>
  Handle(maelstrom::Network::Session session,
         maelstrom::Request request) override {
    auto body = nlohmann::json({});
    body["echo"] = request.body["echo"];
    return yaclib::MakeFuture(std::move(request).ToResponse(std::move(body)));
  }
};

} // namespace tasks::echo