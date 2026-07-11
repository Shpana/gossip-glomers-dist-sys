#pragma once

#include <exception>
#include <memory>
#include <string_view>
#include <utility>
#include <vector>

#include <yaclib/async/future.hpp>
#include <yaclib/async/run.hpp>
#include <yaclib/runtime/fair_thread_pool.hpp>
#include <yaclib/util/intrusive_ptr.hpp>

#include "environment.hpp"
#include "handler.hpp"
#include "logging.hpp"
#include "network/messages.hpp"
#include "network/network.hpp"
#include "network/transport.hpp"

namespace ds::core {
  template<typename State>
  class Node {
    static constexpr std::string_view kInit = "init";

  public:
    Node();

    void run();

    template<typename Handler>
    void add();

  private:
    bool start();
    void handle(Request&& request);
    void handle(Response&& response);
    void stop();

  private:
    yaclib::IntrusivePtr<yaclib::FairThreadPool> thread_pool_;

    std::shared_ptr<Transport> transport_;
    std::optional<Environment<State>> env_{std::nullopt};

    std::shared_ptr<Network> network_;

    bool was_started_{false};
    std::unordered_map<std::string, std::unique_ptr<HandlerBase<State>>>
        handlers_{};
  };

  template<typename State>
  Node<State>::Node()
      : thread_pool_{yaclib::MakeFairThreadPool(2)},
        transport_{std::make_shared<Transport>()},
        network_{std::make_shared<Network>(thread_pool_, transport_)} {}

  template<typename State>
  void Node<State>::run() {
    if (start()) {
      LOG_INFO() << "Node successfully started!\n";

      while (transport_->isStreaming()) {
        auto message = transport_->recieve();
        if (!message.has_value()) {
          continue;
        }

        if (message.value().isRequest()) {
          handle(std::move(message.value()).toRequest().value());
          continue;
        } else if (message.value().isResponse()) {
          handle(std::move(message.value()).toResponse().value());
          continue;
        }

        LOG_ERROR() << "Unknown type of message!\n";
      }
    }

    stop();
  }

  template<typename State>
  template<typename Handler>
  void Node<State>::add() {
    auto handler = std::make_unique<Handler>();
    handlers_[std::string{Handler::type}] = std::move(handler);
    LOG_INFO() << fmt::format("Handler '{}' was added to node registry!\n",
                              Handler::type);
  }

  template<typename State>
  bool Node<State>::start() {
    transport_->start();

    auto message = transport_->recieve();
    if (!message.has_value()) {
      LOG_ERROR() << "Failed to start node: cannot parse message!\n";
      return false;
    }

    auto init_request = std::move(message.value()).toRequest();
    if (!init_request.has_value() || init_request.value().type != kInit) {
      LOG_ERROR() << "Failed to start node: errors in init's request!\n";
      return false;
    }

    network_->start();

    const auto& body = init_request.value().body;
    env_.emplace(Environment<State>{
        .node_id = body["node_id"].get<std::string>(),
        .available_node_ids = body["node_ids"].get<std::vector<std::string>>(),
        .state = std::make_shared<State>()});

    for (auto& [type, handler]: handlers_) {
      handler->startInternal(env_.value());
      handler->start();
    }

    std::exchange(was_started_, true);

    transport_->send(std::move(init_request.value()).toResponse().toMessage());
    return true;
  }

  template<typename State>
  void Node<State>::handle(Request&& request) {
    const auto& type = request.type;

    if (auto it = handlers_.find(type); it != handlers_.end()) {
      auto f = yaclib::Run(*thread_pool_, [this, type, it,
                                           request =
                                               std::move(request)]() mutable {
        auto session = network_->makeSession(env_->node_id);

        Response response;

        try {
          const auto& handler = it->second;
          auto f = handler->handle(std::move(session), std::move(request));
          response = std::move(std::move(f).Get().Ok());
        } catch (const std::exception& ex) {
          LOG_ERROR() << fmt::format(
              "There is error, when handling '{}', ex: {}\n", type, ex.what());
        }

        transport_->send(std::move(response).toMessage());
      });
      return;
    }

    LOG_ERROR() << fmt::format("No handler for message of type '{}'\n", type);
  }

  template<typename State>
  void Node<State>::handle(Response&& response) {
    network_->handle(std::move(response));
  }

  template<typename State>
  void Node<State>::stop() {
    if (was_started_) {
      for (auto& [type, handler]: handlers_) {
        handler->stop();
        handler->stopInternal();
      }
    }

    network_->stop();

    transport_->stop();

    thread_pool_->SoftStop();
    thread_pool_->Wait();
  }
}// namespace ds::core