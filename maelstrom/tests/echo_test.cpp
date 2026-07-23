#include <gtest/gtest.h>

#include <memory>

#include <yaclib/async/make.hpp>

#include "detail/network/in_memory_transport.hpp"
#include "network/messages.hpp"
#include "node.hpp"
#include "routines/handler.hpp"
#include "utils/unit.hpp"

namespace maelstrom::tests {
  class EchoHandler final : public HandlerBase<Unit> {
  public:
    static constexpr std::string_view type = "echo";

    yaclib::Future<Response> handle(Network::Session session,
                                    Request request) override {
      auto echo = nlohmann::json({});
      echo["message"] = request.body["message"].get<std::string>();
      return yaclib::MakeFuture(std::move(request).toResponse(std::move(echo)));
    }
  };
}// namespace maelstrom::tests

class EchoTest : public ::testing::Test {
public:
  std::shared_ptr<maelstrom::InMemoryTransport> getTransport() {
    return transport_;
  }

  [[nodiscard]] bool hasNotCatchedExceptions() const {
    return has_not_catched_exceptions_.load();
  }

protected:
  void SetUp() override {
    transport_ = std::make_shared<maelstrom::InMemoryTransport>();

    node_ = std::make_shared<maelstrom::Node<maelstrom::Unit>>();
    node_->add<maelstrom::tests::EchoHandler>();
    node_->useTransport(transport_);

    assistant_ = std::thread{[this, node = node_]() {
      try {
        node->run();
      } catch (...) {
        has_not_catched_exceptions_.store(true);
      }
    }};

    while (!transport_->isRunning()) {
      ;
    }

    // Send inital message
    {
      auto request = maelstrom::Request{
          .source = "c0",
          .destination = "n0",
          .type = "init",
          .body = R"({"node_id": "n0", "node_ids": ["n0"]})"_json,
          .message_id = 0};

      transport_->push(std::move(request).toMessage());
      std::ignore = transport_->pop();
    }
  }

  void TearDown() override {
    transport_->stopStreaming();

    if (assistant_.joinable()) {
      assistant_.join();
    }

    EXPECT_TRUE(transport_->hasNoInflightResponses());
    EXPECT_FALSE(hasNotCatchedExceptions());

    transport_.reset();
    node_.reset();
  }

private:
  std::shared_ptr<maelstrom::InMemoryTransport> transport_;

  std::shared_ptr<maelstrom::Node<maelstrom::Unit>> node_;
  std::thread assistant_;

  std::atomic<bool> has_not_catched_exceptions_{false};
};

TEST_F(EchoTest, RequestOk) {
  auto transport = getTransport();

  auto request = maelstrom::Request{.source = "c0",
                                    .destination = "n0",
                                    .type = "echo",
                                    .body = R"({"message": "42"})"_json,
                                    .message_id = 0};

  transport->push(std::move(request).toMessage());

  auto message = transport->pop();
  EXPECT_TRUE(message.has_value());
  EXPECT_TRUE(message.value().isResponse());

  auto response = std::move(message.value()).toResponse().value();
  EXPECT_EQ(response.source, "n0");
  EXPECT_EQ(response.destination, "c0");
  EXPECT_EQ(response.type, "echo_ok");
  EXPECT_EQ(response.in_reply_to, 0);

  EXPECT_TRUE(response.body.contains("message"));
  EXPECT_EQ(response.body["message"].get<std::string>(), "42");
}

TEST_F(EchoTest, RequestFailWrongType) {
  auto transport = getTransport();

  auto request = maelstrom::Request{.source = "c0",
                                    .destination = "n0",
                                    .type = "some_wrong_type",
                                    .body = R"({"message": "42"})"_json,
                                    .message_id = 0};

  transport->push(std::move(request).toMessage());
}

TEST_F(EchoTest, ManyRequests) {
  auto transport = getTransport();

  constexpr std::size_t count = 10'000;

  for (std::size_t i = 0; i < count; ++i) {
    auto body = nlohmann::json({});
    body["message"] = std::to_string(i);

    auto request = maelstrom::Request{.source = "c0",
                                      .destination = "n0",
                                      .type = "echo",
                                      .body = std::move(body),
                                      .message_id = i};

    transport->push(std::move(request).toMessage());
  }

  std::vector<std::size_t> echos;
  echos.reserve(count);

  for (std::size_t i = 0; i < count; ++i) {
    auto message = transport->pop();
    EXPECT_TRUE(message.has_value());
    EXPECT_TRUE(message.value().isResponse());

    auto response = std::move(message.value()).toResponse().value();
    echos.push_back(std::stoull(response.body["message"].get<std::string>()));
  }

  EXPECT_EQ(echos.size(), count);

  std::sort(echos.begin(), echos.end());
  for (std::size_t i = 0; i < count; ++i) {
    EXPECT_EQ(echos[i], i);
  }
}
