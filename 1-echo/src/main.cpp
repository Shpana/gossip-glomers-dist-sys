#include "echo.hpp"

#include "node.hpp"
#include "unit.hpp"

int main() {
  ds::core::Node<ds::core::Unit> node;
  node.add<ds::echo::EchoHandler>();
  node.run();
}