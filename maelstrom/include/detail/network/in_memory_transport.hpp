#pragma once

#include "detail/sync/queue.hpp"
#include "detail/network/transport.hpp"

namespace maelstrom {
  class InMemoryTransport final : public ITransport {
  public:
    void start() override;
    void stop() override;

    void send(Message message) override;
    std::optional<Message> recieve() override;

    [[nodiscard]] bool isRunning() const override;
    [[nodiscard]] bool isStreaming() const override;

    void startStreaming();
    void stopStreaming();

    void push(Message message);
    std::optional<Message> pop();

    [[nodiscard]] std::size_t infligthResponses() const;
    [[nodiscard]] bool hasInflightResponses() const;
    [[nodiscard]] bool hasNoInflightResponses() const;

  private:
    std::atomic<bool> is_running_{false};
    std::atomic<bool> end_of_stream_{true};

    detail::UnboundedBlockingQueue<Message> in_;
    detail::UnboundedBlockingQueue<Message> out_;
  };
}// namespace maelstrom