#pragma once

#include <yaclib/coro/mutex.hpp>

#include <maelstrom/network/network.hpp>

namespace tasks::gcounter {

struct State {
  using Clock = maelstrom::Network::Clock;
  struct {
    std::atomic<std::uint64_t> value{0};
  } local;

  struct {
    std::atomic<bool> enabled{true};
    yaclib::Mutex<> mtx;
    std::uint64_t value{0};
    std::optional<Clock::time_point> valid_to;
  } cache;
};

} // namespace tasks::gcounter