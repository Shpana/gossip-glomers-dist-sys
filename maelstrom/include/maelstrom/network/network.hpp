#pragma once

#include <chrono>

#include <nlohmann/json.hpp>
#include <yaclib/async/future.hpp>

#include <maelstrom/detail/processors/network.hpp>
#include <maelstrom/environment.hpp>
#include <maelstrom/network/messages.hpp>

namespace maelstrom {

class Network {
public:
  class Session;

  using Clock = std::chrono::steady_clock;

public:
  explicit Network(detail::NetworkProcessor &processor);

  void start(Environment env);
  void stop();

  [[nodiscard]] Session makeSession();

private:
  [[nodiscard]] std::uint64_t generateNextId();

private:
  detail::NetworkProcessor &processor_;
  Environment env_;

  std::atomic<std::uint64_t> previous_id_{0};
  std::uint64_t jitter_;
};

class Network::Session {
public:
  explicit Session(Network &network);

  // Non-copyable
  Session(const Session &) = delete;
  Session &operator=(const Session &) = delete;
  // Movable
  Session(Session &&) = default;
  // Non move-assignable
  Session &operator=(Session &&) = delete;

  ~Session() = default;

  void send(std::string type, std::string destination, nlohmann::json body);
  void sendAtLeastOnce(std::string type, std::string destination,
                       nlohmann::json body);
  yaclib::Future<Response>
  call(std::string type, std::string destination, nlohmann::json body,
       std::optional<Network::Clock::duration> timeout = std::nullopt);
  yaclib::Future<Response> callAtLeastOnce(
      std::string type, std::string destination, nlohmann::json body,
      std::optional<Network::Clock::duration> timeout = std::nullopt);

  template <typename Handler>
  void send(std::string destination, nlohmann::json body);
  template <typename Handler>
  void sendAtLeastOnce(std::string destination, nlohmann::json body);
  template <typename Handler>
  yaclib::Future<Response>
  call(std::string destination, nlohmann::json body,
       std::optional<Network::Clock::duration> timeout = std::nullopt);
  template <typename Handler>
  yaclib::Future<Response> callAtLeastOnce(
      std::string destination, nlohmann::json body,
      std::optional<Network::Clock::duration> timeout = std::nullopt);

private:
  [[nodiscard]] Request makeRequest(std::string type, std::string destination,
                                    nlohmann::json body) const;

private:
  Network &network_;
};

} // namespace maelstrom

template <typename Handler>
void maelstrom::Network::Session::send(std::string destination,
                                       nlohmann::json body) {
  send(std::string{Handler::type}, std::move(destination), std::move(body));
}

template <typename Handler>
void maelstrom::Network::Session::sendAtLeastOnce(std::string destination,
                                                  nlohmann::json body) {
  return sendAtLeastOnce(std::string{Handler::type}, std::move(destination),
                         std::move(body));
}

template <typename Handler>
yaclib::Future<maelstrom::Response> maelstrom::Network::Session::Session::call(
    std::string destination, nlohmann::json body,
    std::optional<Network::Clock::duration> timeout) {
  return call(std::string{Handler::type}, std::move(destination),
              std::move(body), timeout);
}

template <typename Handler>
yaclib::Future<maelstrom::Response>
maelstrom::Network::Session::Session::callAtLeastOnce(
    std::string destination, nlohmann::json body,
    std::optional<Network::Clock::duration> timeout) {
  return callAtLeastOnce(std::string{Handler::type}, std::move(destination),
                         std::move(body), timeout);
}