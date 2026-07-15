#include "node.hpp"

#include "unique_id.hpp"

int main() {
  maelstrom::Node<maelstrom::Unit> node;
  node.add<ds::unique_id::GenerateHandler>();
  node.run();
}