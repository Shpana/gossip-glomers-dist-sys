#include "handlers.hpp"
#include "state.hpp"

#include <maelstrom/node.hpp>
#include <maelstrom/utils/unit.hpp>

int main() {
  maelstrom::Node<maelstrom::Unit> node{};
  node.Add<tasks::kafka::part2::SendHandler>();
  node.Add<tasks::kafka::part2::PollHandler>();
  node.Add<tasks::kafka::part2::CommitOffsetsHandler>();
  node.Add<tasks::kafka::part2::ListCommittedOffsetsHandler>();
  node.Run();
}