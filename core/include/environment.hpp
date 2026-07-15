#pragma once

#include <string>
#include <vector>

namespace ds::core {
  struct Environment {
    std::string node_id;
    std::vector<std::string> available_node_ids;
  };
}// namespace ds::core