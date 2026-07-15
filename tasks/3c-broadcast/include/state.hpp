#pragma once

#include <cstdint>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace ds::broadcast {
  struct State {
    std::mutex mtx_{};
    std::vector<std::uint64_t> nums;// Guarded by mtx_;
    std::optional<std::unordered_map<std::string, std::vector<std::string>>>
        topology{std::nullopt};
  };
}// namespace ds::broadcast