#include "node.hpp"

#include "ops.hpp"

int main() {
  maelstrom::Node<maelstrom::Unit> node{};
  node.add<ds::gcounter::AddHandler>();
  node.add<ds::gcounter::ReadHandler>();
  node.run();
}