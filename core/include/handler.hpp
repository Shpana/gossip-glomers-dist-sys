#pragma once

#include <optional>

#include <yaclib/async/future.hpp>

#include "environment.hpp"
#include "network/messages.hpp"
#include "network/network.hpp"

namespace ds::core {
  template<typename State>
  class Node;

  template<typename State>
  class HandlerBase {
  public:
    virtual ~HandlerBase() = default;

    virtual void start() {}
    virtual yaclib::Future<Response> handle(Network::Session&& session,
                                            Request&& request) = 0;
    virtual void stop() {}

  protected:
    std::optional<Environment<State>> env_{std::nullopt};

  private:
    friend Node<State>;

    void startInternal(Environment<State> env) { env_.emplace(std::move(env)); }
    void stopInternal() {}
  };
}// namespace ds::core