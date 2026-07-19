#include "node.hpp"

#include "handlers.hpp"
#include "state.hpp"

int main() {
  maelstrom::Node<tasks::kafka::part2::State> node{};
  node.add<tasks::kafka::part2::SendHandler>();
  node.add<tasks::kafka::part2::PollHandler>();
  node.add<tasks::kafka::part2::CommitOffsetsHandler>();
  node.add<tasks::kafka::part2::ListCommittedOffsetsHandler>();
  node.run();
}