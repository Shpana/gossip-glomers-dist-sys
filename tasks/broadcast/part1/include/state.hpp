#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace tasks::broadcast::part1 {

struct State {
  std::vector<std::uint64_t> nums;
  std::optional<std::unordered_map<std::string, std::vector<std::string>>>
    topology{std::nullopt};
};

} // namespace tasks::broadcast