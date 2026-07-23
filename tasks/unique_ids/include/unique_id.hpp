#pragma once

#include <yaclib/async/make.hpp>

#include <maelstrom/routines/handler.hpp>
#include <maelstrom/utils/unit.hpp>

namespace tasks::unique_id {

class GenerateHandler final : public maelstrom::HandlerBase<maelstrom::Unit> {
public:
  static constexpr std::string_view kType = "generate";

  void Start() override {
    auto env = GetEnvironment();
    n_ = std::stoll(std::string{env.node_id.begin() + 1, env.node_id.end()});
  }

  yaclib::Future<maelstrom::Response>
  Handle(maelstrom::Network::Session session,
         maelstrom::Request request) override {
    auto body = nlohmann::json({});
    body["id"] = 1000 * ++local_counter_ + n_;

    return yaclib::MakeFuture(std::move(request).ToResponse(std::move(body)));
  }

private:
  std::uint64_t local_counter_{1};
  std::uint64_t n_{};
};

} // namespace tasks::unique_id