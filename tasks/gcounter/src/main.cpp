#include "node.hpp"

#include "operations.hpp"

int main() {
  maelstrom::Node<tasks::gcounter::State> node{};
  node.add<tasks::gcounter::AddHandler>();
  node.add<tasks::gcounter::ReadHandler>();
  node.run();
}