#include "echo.hpp"
#include "node.hpp"

int main() {
  ds::core::Node node;
  node.add<ds::echo::EchoHandler>();
  node.run();
}