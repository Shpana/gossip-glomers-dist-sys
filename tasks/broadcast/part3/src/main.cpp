#include "handlers.hpp"

#include <maelstrom/node.hpp>

int main() {
  maelstrom::Node<tasks::broadcast::part3::State> node{};
  node.Add<tasks::broadcast::part3::BroadcastHandler>();
  node.Add<tasks::broadcast::part3::ReadHandler>();
  node.Add<tasks::broadcast::part3::TopologyHandler>();
  node.Run();
}