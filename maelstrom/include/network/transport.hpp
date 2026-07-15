#pragma once

#include <atomic>

#include <fmt/format.h>

#include "logging.hpp"
#include "messages.hpp"

namespace maelstrom {
  class Transport {
  public:
    void start();
    void stop();

    void send(Message&& message);
    std::optional<Message> recieve();

    [[nodiscard]] bool isStreaming() const;

  private:
    std::atomic<bool> is_running_{false};
    std::atomic<bool> end_of_stream_{false};
    std::mutex mtx_{};
  };
}// namespace maelstrom