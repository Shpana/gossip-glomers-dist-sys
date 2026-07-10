#include "echo.hpp"
#include "node.hpp"

int main() {
  ds::core::Node node;
  node.registerHandler(std::make_unique<ds::echo::EchoHandler>());
  node.run();
}