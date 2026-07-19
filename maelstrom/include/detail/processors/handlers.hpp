#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include <yaclib/async/make.hpp>
#include <yaclib/async/run.hpp>
#include <yaclib/coro/await.hpp>
#include <yaclib/coro/future.hpp>
#include <yaclib/exe/executor.hpp>

#include "environment.hpp"
#include "log/logging.hpp"
#include "network/network.hpp"
#include "network/transport/transport.hpp"
#include "routines/handler.hpp"

namespace maelstrom::detail {
  template<typename State>
  class HandlersProcessor {
  public:
    HandlersProcessor(yaclib::IExecutor& executor, ITransport& transport,
                      Network& network);

    template<IsHandler<State> Handler, typename... Args>
    void add(Args&&... args);

    void start(Environment env, std::shared_ptr<State> state);
    void stop();

    void process(Request request);

  private:
    bool is_running_{false};

    yaclib::IExecutor& executor_;

    ITransport& transport_;
    Network& network_;

    std::unordered_map<std::string, std::unique_ptr<HandlerBase<State>>>
        handlers_{};
  };

  template<typename State>
  HandlersProcessor<State>::HandlersProcessor(yaclib::IExecutor& executor,
                                              ITransport& transport,
                                              Network& network)
      : executor_{executor}, transport_{transport}, network_{network} {}

  template<typename State>
  template<IsHandler<State> Handler, typename... Args>
  void HandlersProcessor<State>::add(Args&&... args) {
    if (is_running_) {
      LOG_ERROR() << fmt::format(
          "Cannot add handler '{}', node already started!\n", Handler::type);
      return;
    }

    auto handler = std::make_unique<Handler>(std::forward<Args>(args)...);
    handlers_[std::string{Handler::type}] = std::move(handler);
    LOG_INFO() << fmt::format("Handler '{}' was added to node registry!\n",
                              Handler::type);
  }

  template<typename State>
  void HandlersProcessor<State>::start(Environment env,
                                       std::shared_ptr<State> state) {
    std::exchange(is_running_, true);

    for (auto& [type, handler]: handlers_) {
      handler->startInternal(env, state);
      handler->start();
    }
  }

  template<typename State>
  void HandlersProcessor<State>::stop() {
    if (is_running_) {
      for (auto& [type, handler]: handlers_) {
        handler->stop();
        handler->stopInternal();
      }

      std::exchange(is_running_, false);
    }
  }

  template<typename State>
  void HandlersProcessor<State>::process(Request request) {
    auto type = request.type;

    auto it = handlers_.find(type);
    if (it == handlers_.end()) {
      LOG_ERROR() << fmt::format("No handler for message of type '{}'\n", type);
      return;
    }

    std::ignore = yaclib::Run(
        executor_,
        [this, type, it,
         request = std::move(request)]() mutable -> yaclib::Future<> {
          try {
            auto session = network_.makeSession();

            const auto& handler = it->second;
            auto response = co_await handler->handle(std::move(session),
                                                     std::move(request));

            transport_.send(std::move(response).toMessage());
          } catch (const std::exception& ex) {
            LOG_ERROR() << fmt::format(
                "Exception occurs, when processing handler '{}', ex: {}\n",
                type, ex.what());
          }
        });
  }
}// namespace maelstrom::detail