#include "network/transport/in_memory_transport.hpp"

#include "log/logging.hpp"

namespace maelstrom {
  void InMemoryTransport::start() { is_running_.store(true); }

  void InMemoryTransport::stop() { is_running_.store(false); }

  void InMemoryTransport::send(Message&& message) {
    if (!is_running_.load()) {
      LOG_ERROR() << "Try to send message by not running transport!\n";
      return;
    }
    out_.push(std::move(message));
  }

  std::optional<Message> InMemoryTransport::recieve() {
    if (!is_running_.load()) {
      LOG_ERROR() << "Try to send message by not running transport!\n";
      return std::nullopt;
    }
    return in_.pop();
  }

  void InMemoryTransport::stopStreaming() { in_.close(); }

  [[nodiscard]] bool InMemoryTransport::isStreaming() const {
    return !in_.isClosed();
  }

  [[nodiscard]] bool InMemoryTransport::isRunning() const {
    return is_running_.load();
  }

  void InMemoryTransport::push(Message&& message) {
    if (!is_running_.load()) {
      LOG_ERROR() << "Try to push message to not running transport!\n";
      return;
    }
    in_.push(std::move(message));
  }

  std::optional<Message> InMemoryTransport::pop() {
    if (!is_running_.load()) {
      LOG_ERROR() << "Try to pop message from not running transport!\n";
      return std::nullopt;
    }
    return out_.pop();
  }

  [[nodiscard]] bool InMemoryTransport::hasNoResponses() const {
    return out_.isEmpty();
  }

  [[nodiscard]] std::size_t InMemoryTransport::infligthResponses() const {
    return out_.size();
  }
}// namespace maelstrom