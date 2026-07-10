#pragma once

#include "node.hpp"

namespace ds::echo {
  class EchoHandler final : public core::IHandler {
    static constexpr std::string_view kType = "echo";
    static constexpr std::string_view kTypeOk = "echo_ok";

  public:
    [[nodiscard]] std::string_view type() const override {
      return "echo";
    }

    core::Response handle(core::Request&& request) override {
      auto echo_body = nlohmann::json({});
      echo_body["echo"] = request.body["echo"];
      return request.reply(kTypeOk, std::move(echo_body));
    }
  };
}// namespace ds::echo