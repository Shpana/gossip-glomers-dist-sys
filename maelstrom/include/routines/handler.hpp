#pragma once

#include <concepts>
#include <optional>

#include <yaclib/async/future.hpp>

#include "environment.hpp"
#include "network/messages.hpp"
#include "network/network.hpp"

namespace maelstrom {
  namespace detail {
    template<typename State>
    class HandlersProcessor;
  }

  template<typename State>
  class HandlerBase {
  public:
    virtual ~HandlerBase() = default;

    virtual void start() {}
    virtual yaclib::Future<Response> handle(Network::Session session,
                                            Request request) = 0;
    virtual void stop() {}

  protected:
    std::optional<const Environment> env_{std::nullopt};
    std::shared_ptr<State> state_;

  private:
    friend detail::HandlersProcessor<State>;

    void startInternal(Environment env, std::shared_ptr<State> state);
    void stopInternal();
  };

  template<typename State>
  void HandlerBase<State>::startInternal(Environment env,
                                         std::shared_ptr<State> state) {
    env_.emplace(std::move(env));
    state_ = std::move(state);
  }

  template<typename State>
  void HandlerBase<State>::stopInternal() {}


  template<typename Handler, typename State>
  concept IsHandler = std::derived_from<Handler, HandlerBase<State>> &&
                      requires(Handler handler) {
                        { Handler::type };
                      };
}// namespace maelstrom