#include "echo.hpp"
#include "node.hpp"

int main() {
  maelstrom::Node<maelstrom::Unit> node;
  node.add<ds::echo::EchoHandler>();
  node.run();
}