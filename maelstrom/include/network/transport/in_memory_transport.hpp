#pragma once

#include "detail/sync/queue.hpp"
#include "network/transport/transport.hpp"

namespace maelstrom {
  class InMemoryTransport final : public ITransport {
  public:
    void start() override;
    void stop() override;

    void send(Message message) override;
    std::optional<Message> recieve() override;

    void stopStreaming() override;
    [[nodiscard]] bool isStreaming() const override;

    [[nodiscard]] bool isRunning() const override;

    void push(Message message);
    std::optional<Message> pop();

    // TODO(shpana): better name?
    [[nodiscard]] bool hasNoResponses() const;
    [[nodiscard]] std::size_t infligthResponses() const;

  private:
    std::atomic<bool> is_running_{false};
    std::atomic<bool> end_of_stream_{false};

    detail::UnboundedBlockingQueue<Message> in_;
    detail::UnboundedBlockingQueue<Message> out_;
  };
}// namespace maelstrom