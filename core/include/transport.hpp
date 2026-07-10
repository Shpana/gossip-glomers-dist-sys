#pragma once

#include <iostream>

#include <fmt/format.h>

#include "messages.hpp"

namespace ds::core {
  class Transport {
  public:
    void send(Response&& message) {
      if (!is_running_) {
        return;
      }

      nlohmann::json json_message;
      json_message["src"] = message.sender;
      json_message["dest"] = message.recipient;
      json_message["body"] = message.body;
      std::cout << json_message << std::endl;
    }

    std::optional<Request> recieve() {
      if (!is_running_) {
        return std::nullopt;
      }

      std::string input;
      std::getline(std::cin, input);
      if (input.empty()) {
        std::exchange(is_running_, false);
        return std::nullopt;
      }

      auto request = Request::parse(input);
      if (!request.has_value()) {
        std::cerr << fmt::format("Failed to parse request: {}!\n", input);
        return std::nullopt;
      }

      return request;
    }

    [[nodiscard]] bool isRunning() const {
      return is_running_;
    }

  private: 
    bool is_running_{true};
  };
}// namespace ds::core