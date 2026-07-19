#include "node.hpp"

#include "handlers.hpp"
#include "state.hpp"

int main() {
  maelstrom::Node<tasks::kafka::part1::State> node{};
  node.add<tasks::kafka::part1::SendHandler>();
  node.add<tasks::kafka::part1::PollHandler>();
  node.add<tasks::kafka::part1::CommitOffsetsHandler>();
  node.add<tasks::kafka::part1::ListCommittedOffsetsHandler>();
  node.run();
}