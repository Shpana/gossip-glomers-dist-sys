#include <gtest/gtest.h>

#include <yaclib/async/make.hpp>

#include "node.hpp"
#include "routines/handler.hpp"
#include "utils/unit.hpp"

// TODO(shpana): add stringstream transport

namespace maelstrom::tests {
  class EchoHandler final : public HandlerBase<Unit> {
  public:
    static constexpr std::string_view type = "echo";

    yaclib::Future<Response> handle(Network::Session&& session,
                                    Request&& request) override {
      auto echo = nlohmann::json({});
      echo["message"] = request.body["message"].get<std::string>();
      return yaclib::MakeFuture(std::move(request).toResponse(std::move(echo)));
    }
  };
}// namespace maelstrom::tests

TEST(Echo, InitOk) {
  // ...
}

TEST(Echo, InitFails) {
  // ...
}

TEST(Echo, RequestOk) {
  // ...
}

TEST(Echo, RequestFail) {
  // ...
}

TEST(Echo, ManyRequests) {
  // ...
}

TEST(Echo, NoExceptionsAfterFail) {
  // ...
}

TEST(Echo, NoExceptionsAfterStop) {
  // ...
}