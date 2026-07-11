#include "node.hpp"

#include "handlers/broadcast.hpp"
#include "handlers/read.hpp"
#include "handlers/topology.hpp"

int main() {
  ds::core::Node<ds::broadcast::State> node{};
  node.add<ds::broadcast::BroadcastHandler>();
  node.add<ds::broadcast::ReadHandler>();
  node.add<ds::broadcast::TopologyHandler>();
  node.run();
}