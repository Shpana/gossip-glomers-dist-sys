#pragma once

#include "state.hpp"

#include <maelstrom/routines/worker.hpp>

namespace tasks::broadcast::part4 {

class DistributorWorker final : public maelstrom::WorkerBase<State> {
public:
  static constexpr std::string_view kType = "distributor";

  using maelstrom::WorkerBase<State>::WorkerBase;

  void Start() override;
  yaclib::Future<> Process(maelstrom::Network::Session session) override;

private:
  std::vector<std::string> node_ids_for_notify_{};
};

} // namespace tasks::broadcast::part4