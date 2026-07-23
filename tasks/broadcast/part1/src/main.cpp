#include "handlers.hpp"

#include <maelstrom/node.hpp>

int main() {
  maelstrom::Node<tasks::broadcast::part1::State> node{};
  node.Add<tasks::broadcast::part1::BroadcastHandler>();
  node.Add<tasks::broadcast::part1::ReadHandler>();
  node.Add<tasks::broadcast::part1::TopologyHandler>();
  node.Run();
}