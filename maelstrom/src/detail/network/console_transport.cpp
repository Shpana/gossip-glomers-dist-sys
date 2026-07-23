#include <maelstrom/detail/network/console_transport.hpp>

#include <maelstrom/log/logging.hpp>

namespace maelstrom {

void ConsoleTransport::start() { is_running_.store(true); }

void ConsoleTransport::stop() { is_running_.store(false); }

void ConsoleTransport::send(Message message) {
  if (!is_running_.load()) {
    LOG_ERROR() << "Try to send message by not running transport!\n";
    return;
  }

  auto json = std::move(message).toJson();

  LOG_DEBUG() << "<- " << json << std::endl;

  {
    std::lock_guard guard{mtx_};
    std::cout << json << std::endl;
  }
}

std::optional<Message> ConsoleTransport::recieve() {
  if (!is_running_.load()) {
    LOG_ERROR() << "Try to send message by not running transport!\n";
    return std::nullopt;
  }

  std::string input;
  std::getline(std::cin, input);
  if (input.empty()) {
    end_of_stream_.store(true);
    return std::nullopt;
  }

  LOG_DEBUG() << "-> " << input << std::endl;

  auto message = Message::parse(std::move(input));
  if (!message.has_value()) {
    return std::nullopt;
  }

  return message;
}

[[nodiscard]] bool ConsoleTransport::isRunning() const {
  return is_running_.load();
}

[[nodiscard]] bool ConsoleTransport::isStreaming() const {
  return !end_of_stream_.load();
}

} // namespace maelstrom