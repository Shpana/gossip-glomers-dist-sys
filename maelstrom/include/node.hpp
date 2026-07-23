#pragma once

#include <memory>
#include <string_view>
#include <utility>
#include <vector>

#include <yaclib/coro/future.hpp>
#include <yaclib/runtime/fair_thread_pool.hpp>

#include "detail/network/console_transport.hpp"
#include "detail/network/transport.hpp"
#include "detail/processors/handlers.hpp"
#include "detail/processors/network.hpp"
#include "detail/processors/workers.hpp"
#include "environment.hpp"
#include "log/logging.hpp"
#include "network/network.hpp"

namespace maelstrom {
  template<typename State>
  class Node final : private detail::NetworkProcessor,
                     private detail::HandlersProcessor<State>,
                     private detail::WorkersProcessor<State> {
  public:
    Node();

    void run();

    using detail::HandlersProcessor<State>::add;
    using detail::WorkersProcessor<State>::add;

    void useTransport(std::shared_ptr<ITransport> transport);

  private:
    bool loadEnvironment();
    bool start();
    void stop();

  private:
    yaclib::FairThreadPool cpu_pool_;

    std::shared_ptr<ITransport> transport_ =
        std::make_shared<ConsoleTransport>();
    Network network_;

    std::optional<Environment> env_{std::nullopt};
    std::shared_ptr<State> state_{};
  };

  template<typename State>
  Node<State>::Node()
      : detail::NetworkProcessor{},
        detail::HandlersProcessor<State>{cpu_pool_, network_},
        detail::WorkersProcessor<State>{cpu_pool_, network_},
        cpu_pool_{yaclib::FairThreadPool{1}}, network_{*this} {}

  template<typename State>
  void Node<State>::useTransport(std::shared_ptr<ITransport> transport) {
    transport_ = std::move(transport);
  }

  template<typename State>
  bool Node<State>::loadEnvironment() {
    auto message = transport_->recieve();
    if (!message.has_value()) {
      LOG_ERROR() << "Failed to load envrionment information! Cannot parse "
                     "initialization message!\n";
      return false;
    }

    auto init_request = std::move(message.value()).toRequest();

    constexpr std::string_view init_type = "init";

    if (!init_request.has_value() || init_request.value().type != init_type) {
      LOG_ERROR() << "Failed to load envrionment information! There are errors "
                     "in initializaiton's request!\n";
      return false;
    }

    const auto& body = init_request.value().body;
    if (!body.contains("node_id") || !body.contains("node_ids")) {
      LOG_ERROR() << "Failed to load environment information! There are no "
                     "required fields in request!\n";
      return false;
    }

    env_.emplace(
        Environment{.node_id = body["node_id"].get<std::string>(),
                    .available_node_ids =
                        body["node_ids"].get<std::vector<std::string>>()});

    transport_->send(std::move(init_request.value()).toResponse().toMessage());
    LOG_INFO() << "Successfully load environment!\n";
    return true;
  }

  template<typename State>
  bool Node<State>::start() {
    LOG_INFO() << "Starting!\n";

    transport_->start();

    if (!loadEnvironment()) {
      return false;
    }

    network_.start(env_.value());

    detail::NetworkProcessor::useTransport(transport_);
    detail::HandlersProcessor<State>::useTransport(transport_);

    state_ = std::make_shared<State>();
    detail::NetworkProcessor::start();
    detail::HandlersProcessor<State>::start(env_.value(), state_);
    detail::WorkersProcessor<State>::start(env_.value(), state_);

    LOG_INFO() << "Successfully started!\n";
    return true;
  }

  template<typename State>
  void Node<State>::run() {
    if (start()) {
      while (transport_->isStreaming()) {
        auto raw_message = transport_->recieve();
        if (!raw_message.has_value()) {
          continue;
        }

        auto message = raw_message.value();

        if (message.isRequest()) {
          detail::HandlersProcessor<State>::process(
              std::move(std::move(message).toRequest().value()));
          continue;
        } else if (message.isResponse()) {
          detail::NetworkProcessor::process(
              std::move(std::move(message).toResponse().value()));
          continue;
        }

        LOG_ERROR() << "Unknown type of message!\n";
      }
    }
    stop();
  }

  template<typename State>
  void Node<State>::stop() {
    LOG_INFO() << "Stopping!\n";

    cpu_pool_.SoftStop();
    cpu_pool_.Wait();

    detail::HandlersProcessor<State>::stop();
    detail::WorkersProcessor<State>::stop();

    NetworkProcessor::stop();

    network_.stop();
    transport_->stop();

    LOG_INFO() << "Successfully stopped!\n";
  }
}// namespace maelstrom