#pragma once

#include "network/messages.hpp"
#include "network/network.hpp"
#include "routines/handler.hpp"

#include <yaclib/coro/mutex.hpp>

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

class AddHandler final : public maelstrom::HandlerBase<State> {
public:
  static constexpr std::string_view kType = "add";

  yaclib::Future<maelstrom::Response>
  handle(maelstrom::Network::Session &&session,
         maelstrom::Request &&request) override;
};

class ReadHandler final : public maelstrom::HandlerBase<State> {
public:
  static constexpr std::string_view kType = "read";

  yaclib::Future<maelstrom::Response>
  handle(maelstrom::Network::Session &&session,
         maelstrom::Request &&request) override;

private:
  yaclib::Future<std::uint64_t> Read(maelstrom::Network::Session &session);
};
} // namespace tasks::gcounter