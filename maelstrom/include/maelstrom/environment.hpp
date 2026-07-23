#pragma once

#include <string>
#include <vector>

namespace maelstrom {

struct Environment {
  std::string node_id;
  std::vector<std::string> available_node_ids;
};

} // namespace maelstrom