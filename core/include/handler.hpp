#pragma once

#include <optional>

#include <yaclib/async/future.hpp>

#include "network/messages.hpp"
#include "network/network.hpp"

namespace ds::core {
  template<typename State>
  class Node;

  template<typename State>
  class HandlerBase {
    using Environment = Node<State>::Environment;

  public:
    virtual ~HandlerBase() = default;

    virtual yaclib::Future<Response> handle(Network::Session&& session,
                                            Request&& request) = 0;

  protected:
    std::optional<Environment> env_{std::nullopt};

  private:
    friend Node<State>;

    void start(Environment env) { env_.emplace(std::move(env)); }
    void stop() {}
  };
}// namespace ds::core