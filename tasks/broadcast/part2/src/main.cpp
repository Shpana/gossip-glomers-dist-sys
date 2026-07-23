#include "handlers.hpp"

#include <maelstrom/node.hpp>

int main() {
  maelstrom::Node<tasks::broadcast::part2::State> node{};
  node.Add<tasks::broadcast::part2::BroadcastHandler>();
  node.Add<tasks::broadcast::part2::ReadHandler>();
  node.Add<tasks::broadcast::part2::TopologyHandler>();
  node.Run();
}