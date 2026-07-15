#include "node.hpp"

#include <chrono>
using namespace std::chrono_literals;

#include "handlers/broadcast.hpp"
#include "handlers/read.hpp"
#include "handlers/topology.hpp"

#include "workers/distributor.hpp"

int main() {
  ds::core::Node<ds::broadcast::State> node{};
  node.add<ds::broadcast::BroadcastHandler>();
  node.add<ds::broadcast::BroadcastBulkHandler>();
  node.add<ds::broadcast::ReadHandler>();
  node.add<ds::broadcast::TopologyHandler>();

  node.add<ds::broadcast::DistributorWorker>(500ms);
  node.run();
}