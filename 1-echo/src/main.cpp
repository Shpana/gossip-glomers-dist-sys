#include "echo.hpp"

#include "node.hpp"

int main() {
  ds::core::Node<ds::core::Unit> node;
  node.add<ds::echo::EchoHandler>();
  node.run();
}