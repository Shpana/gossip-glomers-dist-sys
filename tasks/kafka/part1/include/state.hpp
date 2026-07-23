#pragma once

#include <atomic>
#include <cstdint>
#include <map>
#include <string>
#include <unordered_map>

#include <yaclib/coro/shared_mutex.hpp>

namespace tasks::kafka::part1 {

using Offset = std::uint64_t;
using Message = std::uint64_t;

struct State {
  std::atomic<Offset> next_offset{0};

  yaclib::SharedMutex<> mtx;
  std::unordered_map<std::string, Offset> committed; // Guarded by mtx
  std::unordered_map<std::string, std::map<Offset, Message>>
    logs; // Guarde by mtx
};

} // namespace tasks::kafka::part1