#pragma once

#include <iostream>

#include <fmt/format.h>

#include "logging.hpp"
#include "messages.hpp"

namespace ds::core {
  class Transport {
  public:
    void send(Message&& message);
    std::optional<Message> recieve();

    [[nodiscard]] bool isRunning() const;

  private:
    bool is_running_{true};
  };
}// namespace ds::core