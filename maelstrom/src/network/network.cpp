#include "network/network.hpp"

namespace maelstrom {
  Network::Network(detail::NetworkProcessor& processor)
      : processor_{processor} {}

  void Network::start(Environment env) { env_ = std::move(env); }

  void Network::stop() {}

  Network::Session Network::makeSession() { return Session{*this}; }

  Network::Session::Session(Network& network) : network_{network} {}

  void Network::Session::send(std::string type, std::string destination,
                              nlohmann::json body) {
    network_.processor_.send(
        makeRequest(std::move(type), std::move(destination), std::move(body)));
  }

  void Network::Session::sendAtLeastOnce(std::string type,
                                         std::string destination,
                                         nlohmann::json body) {
    network_.processor_.sendAtLeastOnce(
        makeRequest(std::move(type), std::move(destination), std::move(body)));
  }

  yaclib::Future<Response> Network::Session::Session::call(
      std::string type, std::string destination, nlohmann::json body,
      std::optional<Network::Clock::duration> timeout) {
    return network_.processor_.call(
        makeRequest(std::move(type), std::move(destination), std::move(body)),
        timeout);
  }

  yaclib::Future<Response> Network::Session::Session::callAtLeastOnce(
      std::string type, std::string destination, nlohmann::json body,
      std::optional<Network::Clock::duration> timeout) {
    return network_.processor_.callAtLeastOnce(
        makeRequest(std::move(type), std::move(destination), std::move(body)),
        timeout);
  }

  Request Network::Session::makeRequest(std::string type,
                                        std::string destination,
                                        nlohmann::json body) const {
    return Request{.source = network_.env_.node_id,
                   .destination = std::move(destination),
                   .type = std::move(type),
                   .body = std::move(body),
                   .message_id = network_.previous_id_.fetch_add(1)};
  }
}// namespace maelstrom