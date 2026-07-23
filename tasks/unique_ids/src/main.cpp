#include "unique_id.hpp"

#include <maelstrom/node.hpp>

int main() {
  maelstrom::Node<maelstrom::Unit> node;
  node.Add<tasks::unique_id::GenerateHandler>();
  node.Run();
}