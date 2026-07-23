#pragma once

#include "state.hpp"

#include <maelstrom/routines/worker.hpp>

namespace tasks::broadcast::part5 {

class DistributorWorker final : public maelstrom::WorkerBase<State> {
public:
  static constexpr std::string_view kType = "distributor";

  using maelstrom::WorkerBase<State>::WorkerBase;

  yaclib::Future<> Process(maelstrom::Network::Session session) override;
};

} // namespace tasks::broadcast::part5