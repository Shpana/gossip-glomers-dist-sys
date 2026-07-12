#pragma once

#include <yaclib/async/make.hpp>

#include "routines/handler.hpp"

#include "state.hpp"

namespace ds::broadcast {
  class TopologyHandler final : public core::HandlerBase<State> {
  public:
    static constexpr std::string_view type = "topology";

    yaclib::Future<core::Response>
    handle([[maybe_unused]] core::Network::Session&& session,
           core::Request&& request) override {
      env_->state->topology.emplace(
          request.body["topology"]
              .get<
                  std::unordered_map<std::string, std::vector<std::string>>>());

      return yaclib::MakeFuture(std::move(request).toResponse());
    }
  };
}// namespace ds::broadcast