#include "handlers.hpp"
#include "state.hpp"

#include <maelstrom/node.hpp>

int main() {
  maelstrom::Node<tasks::kafka::part1::State> node{};
  node.Add<tasks::kafka::part1::SendHandler>();
  node.Add<tasks::kafka::part1::PollHandler>();
  node.Add<tasks::kafka::part1::CommitOffsetsHandler>();
  node.Add<tasks::kafka::part1::ListCommittedOffsetsHandler>();
  node.Run();
}