#pragma once

#include <memory>
#include <string>
#include <vector>

namespace ds::core {
  template<typename State>
  struct Environment {
    std::string node_id;
    std::vector<std::string> available_node_ids;
    std::shared_ptr<State> state;
  };
}// namespace ds::core