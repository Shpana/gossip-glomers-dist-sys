#include "network/transport.hpp"

#include "logging.hpp"

namespace ds::core {
  void Transport::start() { is_running_.store(true); }

  void Transport::stop() { is_running_.store(false); }

  void Transport::send(Message&& message) {
    if (!is_running_.load()) {
      LOG_ERROR() << "Try to send message by not running transport!\n";
      return;
    }

    nlohmann::json json_message;
    json_message["src"] = std::move(message.source);
    json_message["dest"] = std::move(message.destination);
    json_message["body"] = std::move(message.body);

    LOG_DEBUG() << "<- " << json_message << std::endl;

    {
      std::lock_guard guard{mtx_};
      std::cout << json_message << std::endl;
    }
  }

  std::optional<Message> Transport::recieve() {
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

  [[nodiscard]] bool Transport::isStreaming() const {
    return !end_of_stream_.load();
  }
}// namespace ds::core