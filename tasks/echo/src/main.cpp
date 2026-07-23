#include "echo.hpp"

#include <maelstrom/node.hpp>

int main() {
  maelstrom::Node<maelstrom::Unit> node;
  node.Add<tasks::echo::EchoHandler>();
  node.Run();
}