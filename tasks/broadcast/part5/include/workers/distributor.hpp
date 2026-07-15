#pragma once

#include "routines/worker.hpp"

#include "state.hpp"

namespace ds::broadcast {
  class DistributorWorker final : public maelstrom::WorkerBase<State> {
  public:
    static constexpr std::string_view type = "distributor";

    using maelstrom::WorkerBase<State>::WorkerBase;

    yaclib::Future<>
    process([[maybe_unused]] maelstrom::Network::Session&& session) override;
  };
}// namespace ds::broadcast