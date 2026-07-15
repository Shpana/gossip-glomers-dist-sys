#pragma once

#include "routines/worker.hpp"

#include "state.hpp"

namespace ds::broadcast {
  class DistributorWorker final : public core::WorkerBase<State> {
  public:
    static constexpr std::string_view type = "distributor";

    using core::WorkerBase<State>::WorkerBase;

    yaclib::Future<>
    process([[maybe_unused]] core::Network::Session&& session) override;
  };
}// namespace ds::broadcast