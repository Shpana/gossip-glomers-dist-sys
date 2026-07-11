#include "network/transport.hpp"

#include "logging.hpp"

namespace ds::core {
  void Transport::send(Message&& message) {
    if (!is_running_) {
      LOG_ERROR() << "Try to send message by not running transport!\n";
      return;
    }

    nlohmann::json json_message;
    json_message["src"] = message.source;
    json_message["dest"] = message.destination;
    json_message["body"] = message.body;
    std::cout << json_message << std::endl;
  }

  std::optional<Message> Transport::recieve() {
    if (!is_running_) {
      LOG_ERROR() << "Try to send message by not running transport!\n";
      return std::nullopt;
    }

    std::string input;
    std::getline(std::cin, input);
    if (input.empty()) {
      std::exchange(is_running_, false);
      return std::nullopt;
    }

    auto message = Message::parse(std::move(input));
    if (!message.has_value()) {
      return std::nullopt;
    }

    return message;
  }

  [[nodiscard]] bool Transport::isRunning() const { return is_running_; }
}// namespace ds::core