#pragma once

#include <memory>
#include <string>
#include <vector>

namespace ds::core {
  template<typename State>
  struct Environment {
    const std::string node_id;
    const std::vector<std::string> available_node_ids;
    const std::shared_ptr<State> state;
  };
}// namespace ds::core