#pragma once

#include <atomic>

#include "network/transport.hpp"

namespace maelstrom {

  class ConsoleTransport final : public ITransport {
  public:
    void start() override;
    void stop() override;

    void send(Message&& message) override;
    std::optional<Message> recieve() override;

    void stopStreaming() override;
    [[nodiscard]] bool isStreaming() const override;

    [[nodiscard]] bool isRunning() const override;

  private:
    std::mutex mtx_{};
    std::atomic<bool> is_running_{false};
    std::atomic<bool> end_of_stream_{false};
  };
}// namespace maelstrom