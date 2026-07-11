#pragma once

#include "node.hpp"

namespace ds::unique_id {
  class GenerateHandler final : public core::IHandler {
    static constexpr std::string_view kType = "generate";
    static constexpr std::string_view kTypeOk = "generate_ok";

  public:
    [[nodiscard]] std::string_view type() const override {
      return kType;
    }

    core::Response handle(core::Request&& request) override {
      auto body = nlohmann::json({});
      body["id"] = 100 * ++local_counter_ + n_;
      return request.reply(kTypeOk, std::move(body));
    }

    void start(core::Environment env) override {
      std::string raw_index{env.node_id.begin() + 1, env.node_id.end()};
      n_ = std::stoll(raw_index);
    }

  private:
    std::uint64_t local_counter_{1};
    std::uint64_t n_;
  };
}// namespace ds::unique_id