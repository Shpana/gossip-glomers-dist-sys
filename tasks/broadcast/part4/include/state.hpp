#pragma once

#include <atomic>
#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace tasks::broadcast::part4 {

struct State {
  struct {
    std::mutex mtx{};
    std::unordered_set<std::uint64_t> unique_nums{}; // Guarded by mtx
    std::vector<std::uint64_t> nums{};               // Guarded by mtx

    using Offset = std::vector<std::uint64_t>::difference_type;

    std::unordered_map<std::string, Offset> maybe_unknown{}; // Guarded by mtx
  } storage;

  struct {
    std::atomic<bool> ready{false};

    std::mutex mtx{};
    std::vector<std::string> all_node_ids{}; // Guarded by mtx
    std::unordered_map<std::string, std::vector<std::string>>
      topology{}; // Guarded by mtx
  } info;
};

} // namespace tasks::broadcast::part4