#pragma once

#include <yaclib/async/make.hpp>

#include "routines/handler.hpp"
#include "utils/unit.hpp"

namespace ds::unique_id {
  class GenerateHandler final : public maelstrom::HandlerBase<maelstrom::Unit> {
  public:
    static constexpr std::string_view type = "generate";

    void start() override {
      std::string raw_index{env_->node_id.begin() + 1, env_->node_id.end()};
      n_ = std::stoll(raw_index);
    }

    yaclib::Future<maelstrom::Response>
    handle([[maybe_unused]] maelstrom::Network::Session&& session,
           maelstrom::Request&& request) override {
      auto body = nlohmann::json({});
      body["id"] = 100 * ++local_counter_ + n_;
      return yaclib::MakeFuture(std::move(request).toResponse(std::move(body)));
    }

  private:
    std::uint64_t local_counter_{1};
    std::uint64_t n_;
  };
}// namespace ds::unique_id