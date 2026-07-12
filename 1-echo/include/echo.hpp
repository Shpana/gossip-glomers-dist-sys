#pragma once

#include <yaclib/async/make.hpp>

#include "routines/handler.hpp"
#include "utils/unit.hpp"

namespace ds::echo {
  class EchoHandler final : public core::HandlerBase<core::Unit> {
  public:
    static constexpr std::string_view type = "echo";

    yaclib::Future<core::Response>
    handle([[maybe_unused]] core::Network::Session&& session,
           core::Request&& request) override {
      auto body = nlohmann::json({});
      body["echo"] = request.body["echo"];
      return yaclib::MakeFuture(std::move(request).toResponse(std::move(body)));
    }
  };
}// namespace ds::echo