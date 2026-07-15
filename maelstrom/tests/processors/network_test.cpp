#include <gtest/gtest.h>

#include <chrono>
#include <thread>

#include "detail/processors/network.hpp"
#include "network/in_memory_transport.hpp"
#include "network/messages.hpp"

using namespace std::chrono_literals;

class NetworkProcessorTest : public ::testing::Test {
public:
  [[nodiscard]] std::shared_ptr<maelstrom::InMemoryTransport>
  getTransport() const {
    return transport_;
  }

  [[nodiscard]] std::shared_ptr<maelstrom::detail::NetworkProcessor>
  getProcessor() const {
    return processor_;
  }

protected:
  void SetUp() override {
    transport_ = std::make_shared<maelstrom::InMemoryTransport>();

    // TODO(shpana): configure frequency of repeating?
    processor_ =
        std::make_shared<maelstrom::detail::NetworkProcessor>(*transport_);

    transport_->start();
  }

  void TearDown() override {
    transport_->stop();

    transport_.reset();
    processor_.reset();
  }

private:
  std::shared_ptr<maelstrom::InMemoryTransport> transport_;
  std::shared_ptr<maelstrom::detail::NetworkProcessor> processor_;
};

TEST_F(NetworkProcessorTest, StartStopWithoutExceptions) {
  auto processor = getProcessor();
  EXPECT_NO_THROW(processor->start());
  EXPECT_NO_THROW(processor->stop());
}

TEST_F(NetworkProcessorTest, SendOnce) {
  auto transport = getTransport();

  auto processor = getProcessor();
  processor->start();

  {
    auto request =
        maelstrom::Request{.source = "n0",
                           .destination = "n1",
                           .type = "some_info_for_other_node",
                           .body = R"({"some_field_with_info": "67"})"_json,
                           .message_id = 10};

    processor->send(std::move(request));
  }

  {
    auto message = transport->pop();
    EXPECT_TRUE(message.has_value());
    EXPECT_TRUE(message.value().isRequest());

    auto request = std::move(message.value()).toRequest().value();
    EXPECT_EQ(request.source, "n0");
    EXPECT_EQ(request.destination, "n1");
    EXPECT_EQ(request.type, "some_info_for_other_node");
    EXPECT_TRUE(request.body.contains("some_field_with_info"));
    EXPECT_EQ(request.body["some_field_with_info"].get<std::string>(), "67");
    EXPECT_EQ(request.message_id, 10);
  }

  processor->stop();

  EXPECT_TRUE(transport->hasNoResponses());
}

TEST_F(NetworkProcessorTest, SendAtLeastOnceNoTimeout) {
  auto transport = getTransport();

  auto processor = getProcessor();
  processor->start();

  {
    auto request =
        maelstrom::Request{.source = "n0",
                           .destination = "n1",
                           .type = "some_info_for_other_node",
                           .body = R"({"some_field_with_info": "67"})"_json,
                           .message_id = 10};

    processor->sendAtLeastOnce(std::move(request));
  }

  {
    std::this_thread::sleep_for(10s); 

    auto response = maelstrom::Response{.source = "n1",
                                        .destination = "n0",
                                        .type = "some_info_for_other_node_ok",
                                        .body = nlohmann::json({}),
                                        .in_reply_to = 10};

    processor->process(std::move(response));
  }

  processor->stop();
}

TEST_F(NetworkProcessorTest, SendAtLeastOnceBeforeTimeout) {
  auto transport = getTransport();

  auto processor = getProcessor();
  processor->start();

  {
    auto request =
        maelstrom::Request{.source = "n0",
                           .destination = "n1",
                           .type = "some_info_for_other_node",
                           .body = R"({"some_field_with_info": "67"})"_json,
                           .message_id = 10};

    processor->sendAtLeastOnce(std::move(request), 5s);
  }

  {
    std::this_thread::sleep_for(4s);

    EXPECT_GE(transport->infligthResponses(), 4);
  }

  {
    auto response = maelstrom::Response{.source = "n1",
                                        .destination = "n0",
                                        .type = "some_info_for_other_node_ok",
                                        .body = nlohmann::json({}),
                                        .in_reply_to = 10};

    processor->process(std::move(response));
  }

  processor->stop();
}

TEST_F(NetworkProcessorTest, SendAtLeastOnceExpiredTimeout) {
  auto transport = getTransport();

  auto processor = getProcessor();
  processor->start();

  {
    auto request =
        maelstrom::Request{.source = "n0",
                           .destination = "n1",
                           .type = "some_info_for_other_node",
                           .body = R"({"some_field_with_info": "67"})"_json,
                           .message_id = 10};

    processor->sendAtLeastOnce(std::move(request), 5s);
  }

  {
    std::this_thread::sleep_for(6s);

    EXPECT_GE(transport->infligthResponses(), 4);
  }

  processor->stop();
}

TEST_F(NetworkProcessorTest, CallOnceNoTimeout) {
  auto transport = getTransport();

  auto processor = getProcessor();
  processor->start();

  yaclib::Future<maelstrom::Response> f;

  {
    auto request =
        maelstrom::Request{.source = "n0",
                           .destination = "n1",
                           .type = "some_info_for_other_node",
                           .body = R"({"some_field_with_info": "67"})"_json,
                           .message_id = 10};

    f = processor->call(std::move(request));
  }

  {
    std::this_thread::sleep_for(10s);

    auto response = maelstrom::Response{.source = "n1",
                                        .destination = "n0",
                                        .type = "some_info_for_other_node_ok",
                                        .body = nlohmann::json({}),
                                        .in_reply_to = 10};

    processor->process(std::move(response));
  }

  {
    EXPECT_TRUE(f.Ready());
    auto result = std::move(f).Get();
    EXPECT_TRUE(result);

    auto response = std::move(result).Ok();
    EXPECT_EQ(response.type, "some_info_for_other_node_ok");
    EXPECT_EQ(response.in_reply_to, 10);
  }

  processor->stop();
}

TEST_F(NetworkProcessorTest, CallOnceBeforeTimeout) {
  auto transport = getTransport();

  auto processor = getProcessor();
  processor->start();

  yaclib::Future<maelstrom::Response> f;

  {
    auto request =
        maelstrom::Request{.source = "n0",
                           .destination = "n1",
                           .type = "some_info_for_other_node",
                           .body = R"({"some_field_with_info": "67"})"_json,
                           .message_id = 10};

    f = processor->call(std::move(request), 5s);
  }

  {
    std::this_thread::sleep_for(4s);
  }

  {
    auto response = maelstrom::Response{.source = "n1",
                                        .destination = "n0",
                                        .type = "some_info_for_other_node_ok",
                                        .body = nlohmann::json({}),
                                        .in_reply_to = 10};

    processor->process(std::move(response));
  }

  {
    EXPECT_TRUE(f.Ready());
    auto result = std::move(f).Get();
    EXPECT_TRUE(result);

    auto response = std::move(result).Ok();
    EXPECT_EQ(response.type, "some_info_for_other_node_ok");
    EXPECT_EQ(response.in_reply_to, 10);
  }

  processor->stop();
}

TEST_F(NetworkProcessorTest, CallOnceExpiredTimeout) {
  auto transport = getTransport();

  auto processor = getProcessor();
  processor->start();

  yaclib::Future<maelstrom::Response> f;

  {
    auto request =
        maelstrom::Request{.source = "n0",
                           .destination = "n1",
                           .type = "some_info_for_other_node",
                           .body = R"({"some_field_with_info": "67"})"_json,
                           .message_id = 10};

    f = processor->call(std::move(request), 5s);
  }

  {
    std::this_thread::sleep_for(6s);
  }

  {
    EXPECT_TRUE(f.Ready());
    auto result = std::move(f).Get();
    EXPECT_FALSE(result);
    auto exception = std::move(result).Error();

    try {
      std::rethrow_exception(exception);
    } catch (maelstrom::detail::TimeoutException&) {
      // Ok
    } catch (...) {
      // Wrong type
      EXPECT_TRUE(false);
    }
  }

  processor->stop();
}

TEST_F(NetworkProcessorTest, CallAtLeastOnceNoTimeout) {
  auto transport = getTransport();

  auto processor = getProcessor();
  processor->start();

  yaclib::Future<maelstrom::Response> f;

  {
    auto request =
        maelstrom::Request{.source = "n0",
                           .destination = "n1",
                           .type = "some_info_for_other_node",
                           .body = R"({"some_field_with_info": "67"})"_json,
                           .message_id = 10};

    f = processor->callAtLeastOnce(std::move(request));
  }

  {
    std::this_thread::sleep_for(10s);
    EXPECT_GE(transport->infligthResponses(), 8);

    auto response = maelstrom::Response{.source = "n1",
                                        .destination = "n0",
                                        .type = "some_info_for_other_node_ok",
                                        .body = nlohmann::json({}),
                                        .in_reply_to = 10};

    processor->process(std::move(response));
  }

  {
    EXPECT_TRUE(f.Ready());
    auto result = std::move(f).Get();
    EXPECT_TRUE(result);

    auto response = std::move(result).Ok();
    EXPECT_EQ(response.type, "some_info_for_other_node_ok");
    EXPECT_EQ(response.in_reply_to, 10);
  }

  processor->stop();
}

TEST_F(NetworkProcessorTest, CallAtLeastOnceBeforeTimeout) {
  auto transport = getTransport();

  auto processor = getProcessor();
  processor->start();

  yaclib::Future<maelstrom::Response> f;

  {
    auto request =
        maelstrom::Request{.source = "n0",
                           .destination = "n1",
                           .type = "some_info_for_other_node",
                           .body = R"({"some_field_with_info": "67"})"_json,
                           .message_id = 10};

    f = processor->callAtLeastOnce(std::move(request), 5s);
  }

  {
    std::this_thread::sleep_for(4s);
    EXPECT_GE(transport->infligthResponses(), 4);

    auto response = maelstrom::Response{.source = "n1",
                                        .destination = "n0",
                                        .type = "some_info_for_other_node_ok",
                                        .body = nlohmann::json({}),
                                        .in_reply_to = 10};

    processor->process(std::move(response));
  }

  {
    EXPECT_TRUE(f.Ready());
    auto result = std::move(f).Get();
    EXPECT_TRUE(result);

    auto response = std::move(result).Ok();
    EXPECT_EQ(response.type, "some_info_for_other_node_ok");
    EXPECT_EQ(response.in_reply_to, 10);
  }

  processor->stop();
}

TEST_F(NetworkProcessorTest, CallAtLeastOnceExpiredTimeout) {
  auto transport = getTransport();

  auto processor = getProcessor();
  processor->start();

  yaclib::Future<maelstrom::Response> f;

  {
    auto request =
        maelstrom::Request{.source = "n0",
                           .destination = "n1",
                           .type = "some_info_for_other_node",
                           .body = R"({"some_field_with_info": "67"})"_json,
                           .message_id = 10};

    f = processor->callAtLeastOnce(std::move(request), 5s);
  }

  {
    std::this_thread::sleep_for(6s);

    EXPECT_GE(transport->infligthResponses(), 4);
  }

  {
    EXPECT_TRUE(f.Ready());
    auto result = std::move(f).Get();
    EXPECT_FALSE(result);
    auto exception = std::move(result).Error();

    try {
      std::rethrow_exception(exception);
    } catch (maelstrom::detail::TimeoutException&) {
      // Ok
    } catch (...) {
      // Wrong type
      EXPECT_TRUE(false);
    }
  }

  processor->stop();
}

TEST_F(NetworkProcessorTest, ManeCallOnceWithTimeout) {
  auto transport = getTransport();

  auto processor = getProcessor();
  processor->start();

  constexpr std::size_t count = 5'000;
  constexpr std::size_t count_expired = 5'000;

  std::vector<yaclib::Future<maelstrom::Response>> fs;
  fs.reserve(count + count_expired);

  for (std::size_t i = 0; i < count; ++i) {
    auto request =
        maelstrom::Request{.source = "n0",
                           .destination = "n1",
                           .type = "some_info_for_other_node",
                           .body = R"({"some_field_with_info": "67"})"_json,
                           .message_id = i};

    fs.push_back(processor->call(std::move(request), 5s));
  }

  for (std::size_t i = 0; i < count_expired; ++i) {
    auto request =
        maelstrom::Request{.source = "n0",
                           .destination = "n1",
                           .type = "some_info_for_other_node",
                           .body = R"({"some_field_with_info": "67"})"_json,
                           .message_id = i + count_expired};

    fs.push_back(processor->call(std::move(request), 5s));
  }

  std::this_thread::sleep_for(4s);

  for (std::size_t i = 0; i < count; ++i) {
    auto response = maelstrom::Response{.source = "n1",
                                        .destination = "n0",
                                        .type = "some_info_for_other_node_ok",
                                        .body = nlohmann::json({}),
                                        .in_reply_to = i};

    processor->process(std::move(response));
  }

  std::this_thread::sleep_for(2s);

  for (auto&& f: fs) {
    EXPECT_TRUE(f.Ready());
  }

  processor->stop();
}

TEST_F(NetworkProcessorTest, CallOncePingPong) {
  auto transport = getTransport();

  auto processor = getProcessor();
  processor->start();

  constexpr std::size_t iterations = 10'000;

  yaclib::Future<maelstrom::Response> f;

  std::size_t message = 0;
  for (std::size_t i = 0; i < iterations; ++i) {
    {
      auto body = nlohmann::json({});
      body["message"] = message;

      auto request = maelstrom::Request{.source = "n0",
                                        .destination = "n1",
                                        .type = "ping_pong",
                                        .body = std::move(body),
                                        .message_id = i};

      f = processor->call(std::move(request));
    }

    {
      auto message = transport->pop();
      EXPECT_TRUE(message.has_value());
      EXPECT_TRUE(message.value().isRequest());

      auto request = std::move(message.value()).toRequest().value();

      auto body = nlohmann::json({});
      body["message"] = request.body["message"].get<std::size_t>() + 1;

      auto response = std::move(request).toResponse(std::move(body));
      processor->process(std::move(response));
    }

    while (!f.Ready()) {
      ;
    }

    {
      EXPECT_TRUE(f.Ready());
      auto result = std::move(f).Get();
      EXPECT_TRUE(result);

      auto response = std::move(result).Ok();
      message = response.body["message"].get<std::size_t>();
    }
  }

  EXPECT_EQ(message, 10'000);

  processor->stop();
}