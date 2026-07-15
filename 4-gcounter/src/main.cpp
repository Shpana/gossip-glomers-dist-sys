#include "node.hpp"

#include "ops.hpp"

int main() {
  ds::core::Node<ds::core::Unit> node{};
  node.add<ds::gcounter::AddHandler>();
  node.add<ds::gcounter::ReadHandler>();
  node.run();
}