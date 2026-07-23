#pragma once

#include <cstdint>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace tasks::broadcast::part3 {

struct State {
  std::mutex mtx{};
  std::vector<std::uint64_t> nums; // Guarded by mtx
  std::optional<std::unordered_map<std::string, std::vector<std::string>>>
    topology{std::nullopt};
};

} // namespace tasks::broadcast::part3