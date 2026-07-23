#pragma once

#include <maelstrom/network/messages.hpp>

#include <fmt/format.h>

namespace maelstrom {

class ITransport {
public:
  virtual ~ITransport() = default;

  virtual void start() = 0;
  virtual void stop() = 0;

  virtual void send(Message message) = 0;
  virtual std::optional<Message> recieve() = 0;

  [[nodiscard]] virtual bool isRunning() const = 0;
  [[nodiscard]] virtual bool isStreaming() const = 0;
};

} // namespace maelstrom