#include "operations.hpp"

#include <maelstrom/node.hpp>

int main() {
  // TODO(shpana): sometimes read timeouts
  maelstrom::Node<tasks::gcounter::State> node{};
  node.Add<tasks::gcounter::AddHandler>();
  node.Add<tasks::gcounter::ReadHandler>();
  node.Run();
}