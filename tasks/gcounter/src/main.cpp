#include "operations.hpp"

#include <chrono>

#include <maelstrom/node.hpp>

int main() {
  using namespace std::chrono_literals;
  maelstrom::Node<tasks::gcounter::State> node{};
  node.Add<tasks::gcounter::AddHandler>();
  node.Add<tasks::gcounter::ReadHandler>();
  node.Add<tasks::gcounter::OrdererWorker>(1s);
  node.Run();
}