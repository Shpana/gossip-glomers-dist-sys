#pragma once

#include <yaclib/async/make.hpp>

#include "node.hpp"

#include "state.hpp"

namespace ds::broadcast {
  class BroadcastHandler final : public core::HandlerBase<State> {
  public:
    static constexpr std::string_view type = "broadcast";

    yaclib::Future<core::Response> handle(core::Network::Session&& session,
                                          core::Request&& request) override {
      auto num = request.body["message"].get<std::uint64_t>();
      env_->state->nums.push_back(num);
      return yaclib::MakeFuture(std::move(request).toResponse());
    }
  };
}// namespace ds::broadcast