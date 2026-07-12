#pragma once

#include "routines/service.hpp"

#include "state.hpp"

namespace ds::broadcast {
  class DistributorService final : public core::ServiceBase<State> {
  public:
    static constexpr std::string_view type = "distributor";

    using core::ServiceBase<State>::ServiceBase;

    yaclib::Future<core::Unit>
    process([[maybe_unused]] core::Network::Session&& session) override;
  };
}// namespace ds::broadcast