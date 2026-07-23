#include <gtest/gtest.h>

#include <maelstrom/detail/network/in_memory_transport.hpp>
#include <maelstrom/node.hpp>
#include <maelstrom/utils/unit.hpp>

class InitTest : public ::testing::Test {
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
    node_->useTransport(transport_);

    assistant_ = std::thread{[this, node = node_]() {
      try {
        node->run();
      } catch (...) {
        has_not_catched_exceptions_.store(true);
      }
    }};
  }

  void TearDown() override {
    if (assistant_.joinable()) {
      assistant_.join();
    }

    transport_.reset();
    node_.reset();
  }

private:
  std::shared_ptr<maelstrom::InMemoryTransport> transport_;

  std::shared_ptr<maelstrom::Node<maelstrom::Unit>> node_;
  std::thread assistant_;

  std::atomic<bool> has_not_catched_exceptions_{false};
};

TEST_F(InitTest, Ok) {
  auto transport = getTransport();
  while (!transport->isRunning()) {
    ;
  }

  auto request = maelstrom::Request{
      .source = "c0",
      .destination = "n0",
      .type = "init",
      .body = R"({"node_id": "n0", "node_ids": ["n0"]})"_json,
      .message_id = 0};

  transport->push(std::move(request).toMessage());

  auto message = transport->pop();
  EXPECT_TRUE(message.has_value());
  EXPECT_TRUE(message.value().isResponse());

  auto response = std::move(message.value()).toResponse().value();
  EXPECT_EQ(response.source, "n0");
  EXPECT_EQ(response.destination, "c0");
  EXPECT_EQ(response.type, "init_ok");
  EXPECT_EQ(response.in_reply_to, 0);

  transport->stopStreaming();
  while (transport->isRunning()) {
    ;
  }

  EXPECT_TRUE(transport->hasNoInflightResponses());
  EXPECT_FALSE(hasNotCatchedExceptions());
}

TEST_F(InitTest, FailsWrongType) {
  auto transport = getTransport();
  while (!transport->isRunning()) {
    ;
  }

  auto request = maelstrom::Request{
      .source = "c0",
      .destination = "n0",
      .type = "some_wrong_type",
      .body = R"({"node_id": "n0", "node_ids": ["n0"]})"_json,
      .message_id = 0};

  transport->push(std::move(request).toMessage());

  transport->stopStreaming();
  while (transport->isRunning()) {
    ;
  }

  EXPECT_TRUE(transport->hasNoInflightResponses());
  EXPECT_FALSE(hasNotCatchedExceptions());
}

TEST_F(InitTest, FailsWrongBody) {
  auto transport = getTransport();
  while (!transport->isRunning()) {
    ;
  }

  auto request =
      maelstrom::Request{.source = "c0",
                         .destination = "n0",
                         .type = "init",
                         .body = R"({"some_wrong_fields": "wrong_value"})"_json,
                         .message_id = 0};

  transport->push(std::move(request).toMessage());

  transport->stopStreaming();
  while (transport->isRunning()) {
    ;
  }

  EXPECT_TRUE(transport->hasNoInflightResponses());
  EXPECT_FALSE(hasNotCatchedExceptions());
}