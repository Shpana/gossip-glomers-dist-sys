#pragma once

#include <atomic>

#include <maelstrom/detail/network/transport.hpp>

namespace maelstrom {

class ConsoleTransport final : public ITransport {
public:
  void start() override;
  void stop() override;

  void send(Message message) override;
  std::optional<Message> recieve() override;

  [[nodiscard]] bool isRunning() const override;
  [[nodiscard]] bool isStreaming() const override;

private:
  std::mutex mtx_{}; // Guards access to std cout
  std::atomic<bool> is_running_{false};
  std::atomic<bool> end_of_stream_{false};
};

} // namespace maelstrom