#include "handlers/broadcast.hpp"
#include "handlers/read.hpp"
#include "handlers/topology.hpp"
#include "workers/distributor.hpp"

#include <chrono>

#include <maelstrom/node.hpp>

int main() {
  using namespace std::chrono_literals;

  maelstrom::Node<tasks::broadcast::part4::State> node{};
  node.Add<tasks::broadcast::part4::BroadcastHandler>();
  node.Add<tasks::broadcast::part4::BroadcastBulkHandler>();
  node.Add<tasks::broadcast::part4::ReadHandler>();
  node.Add<tasks::broadcast::part4::TopologyHandler>();
  node.Add<tasks::broadcast::part4::DistributorWorker>(500ms);
  node.Run();
}