#pragma once

#include <chrono>

#include <yaclib/async/future.hpp>

#include "detail/processors/network.hpp"
#include "environment.hpp"
#include "messages.hpp"

namespace maelstrom {
  class Network {
  public:
    class Session;

    using Clock = std::chrono::steady_clock;

  public:
    explicit Network(detail::NetworkProcessor& processor);

    void start(Environment env);
    void stop();

    [[nodiscard]] Session makeSession();

  private:
    detail::NetworkProcessor& processor_;
    Environment env_;
    std::atomic<std::uint64_t> previous_id_{0};
  };

  // User-API wrapper for interaction with network
  class Network::Session {
  public:
    explicit Session(Network& network);

    // Non-copyable
    Session(const Session&) = delete;
    Session& operator=(const Session&) = delete;
    // Movable
    Session(Session&&) = default;
    // Non move-assignable
    Session& operator=(Session&&) = delete;

    ~Session() = default;

    template<typename Handler>
    void send(std::string destination, nlohmann::json body);
    template<typename Handler>
    void sendAtLeastOnce(std::string destination, nlohmann::json body);
    template<typename Handler>
    yaclib::Future<Response>
    call(std::string destination, nlohmann::json body,
         std::optional<Network::Clock::duration> timeout = std::nullopt);
    template<typename Handler>
    yaclib::Future<Response> callAtLeastOnce(
        std::string destination, nlohmann::json body,
        std::optional<Network::Clock::duration> timeout = std::nullopt);

  private:
    template<typename Handler>
    [[nodiscard]] Request makeRequest(std::string destination,
                                      nlohmann::json body) const;

  private:
    Network& network_;
  };

  template<typename Handler>
  void Network::Session::send(std::string destination, nlohmann::json body) {
    network_.processor_.send(
        makeRequest<Handler>(std::move(destination), std::move(body)));
  }

  template<typename Handler>
  void Network::Session::sendAtLeastOnce(std::string destination,
                                         nlohmann::json body) {
    network_.processor_.sendAtLeastOnce(
        makeRequest<Handler>(std::move(destination), std::move(body)));
  }

  template<typename Handler>
  yaclib::Future<Response> Network::Session::Session::call(
      std::string destination, nlohmann::json body,
      std::optional<Network::Clock::duration> timeout) {
    return network_.processor_.call(
        makeRequest<Handler>(std::move(destination), std::move(body)), timeout);
  }

  template<typename Handler>
  yaclib::Future<Response> Network::Session::Session::callAtLeastOnce(
      std::string destination, nlohmann::json body,
      std::optional<Network::Clock::duration> timeout) {
    return network_.processor_.callAtLeastOnce(
        makeRequest<Handler>(std::move(destination), std::move(body)), timeout);
  }

  template<typename Handler>
  Request Network::Session::makeRequest(std::string destination,
                                        nlohmann::json body) const {
    return Request{.source = network_.env_.node_id,
                   .destination = std::move(destination),
                   .type = std::string{Handler::type},
                   .body = std::move(body),
                   .message_id = network_.previous_id_.fetch_add(1)};
  }
}// namespace maelstrom