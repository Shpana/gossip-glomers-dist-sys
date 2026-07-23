#include "handlers/broadcast.hpp"
#include "handlers/read.hpp"
#include "handlers/topology.hpp"
#include "workers/distributor.hpp"

#include <chrono>

#include <maelstrom/node.hpp>

int main() {
  using namespace std::chrono_literals;

  maelstrom::Node<tasks::broadcast::part5::State> node{};
  node.Add<tasks::broadcast::part5::BroadcastHandler>();
  node.Add<tasks::broadcast::part5::BroadcastBulkHandler>();
  node.Add<tasks::broadcast::part5::ReadHandler>();
  node.Add<tasks::broadcast::part5::TopologyHandler>();
  node.Add<tasks::broadcast::part5::DistributorWorker>(500ms);
  node.Run();
}