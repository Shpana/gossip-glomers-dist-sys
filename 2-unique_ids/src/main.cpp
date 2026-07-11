#include "node.hpp"
#include "unique_id.hpp"
#include "unit.hpp"

int main() {
  ds::core::Node<ds::core::Unit> node;
  node.add<ds::unique_id::GenerateHandler>();
  node.run();
}