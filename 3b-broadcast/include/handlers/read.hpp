#pragma once

#include <yaclib/async/make.hpp>

#include "routines/handler.hpp"

#include "state.hpp"

namespace ds::broadcast {
  class ReadHandler final : public core::HandlerBase<State> {
  public:
    static constexpr std::string_view type = "read";

    yaclib::Future<core::Response>
    handle([[maybe_unused]] core::Network::Session&& session,
           core::Request&& request) override {
      auto body = nlohmann::json({});
      body["messages"] = env_->state->nums;
      return yaclib::MakeFuture(std::move(request).toResponse(std::move(body)));
    }
  };
}// namespace ds::broadcast