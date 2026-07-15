#include "network/network.hpp"

namespace maelstrom {
  Network::Network(detail::NetworkProcessor& processor)
      : processor_{processor} {}

  void Network::start(Environment env) { env_ = std::move(env); }

  void Network::stop() {}

  Network::Session Network::makeSession() { return Session{*this}; }

  Network::Session::Session(Network& network) : network_{network} {}
}// namespace maelstrom