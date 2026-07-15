#pragma once

#include <memory>
#include <string_view>
#include <utility>
#include <vector>

#include <yaclib/coro/future.hpp>
#include <yaclib/runtime/fair_thread_pool.hpp>

#include "detail/processors/handlers.hpp"
#include "detail/processors/network.hpp"
#include "detail/processors/workers.hpp"
#include "detail/sync/timer.hpp"
#include "environment.hpp"
#include "logging.hpp"
#include "network/network.hpp"
#include "network/transport.hpp"

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

  private:
    void start();
    void loadEnvironment();

    void stop();

  private:
    yaclib::FairThreadPool cpu_pool_;
    detail::Timer background_;

    Transport transport_;
    Network network_;

    std::optional<Environment> env_{std::nullopt};
    std::shared_ptr<State> state_{};
  };

  template<typename State>
  Node<State>::Node()
      : cpu_pool_{yaclib::FairThreadPool{2}},                             //
        background_{cpu_pool_},                                           //
        transport_{},                                                     //
        network_{*this},                                                  //
        detail::NetworkProcessor{background_, transport_},                //
        detail::HandlersProcessor<State>{cpu_pool_, transport_, network_},//
        detail::WorkersProcessor<State>{background_, network_} {}

  template<typename State>
  void Node<State>::start() {
    transport_.start();

    loadEnvironment();

    network_.start(env_.value());

    background_.start();

    state_ = std::make_shared<State>();
    detail::NetworkProcessor::start();
    detail::HandlersProcessor<State>::start(env_.value(), state_);
    detail::WorkersProcessor<State>::start(env_.value(), state_);
  }

  template<typename State>
  void Node<State>::loadEnvironment() {
    auto message = transport_.recieve();
    if (!message.has_value()) {
      LOG_ERROR() << "Failed to load envrionment information!\n";
      throw std::runtime_error{"Cannot parse initialization message!"};
    }

    auto init_request = std::move(message.value()).toRequest();

    constexpr std::string_view init_type = "init";

    if (!init_request.has_value() || init_request.value().type != init_type) {
      LOG_ERROR() << "Failed to load envrionment information!\n";
      throw std::runtime_error{"There are errors in initializaiton's request!"};
    }

    const auto& body = init_request.value().body;
    env_.emplace(
        Environment{.node_id = body["node_id"].get<std::string>(),
                    .available_node_ids =
                        body["node_ids"].get<std::vector<std::string>>()});

    transport_.send(std::move(init_request.value()).toResponse().toMessage());
  }

  template<typename State>
  void Node<State>::run() {
    start();

    LOG_INFO() << "Node successfully started!\n";

    while (transport_.isStreaming()) {
      auto raw_message = transport_.recieve();
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

    stop();
  }

  template<typename State>
  void Node<State>::stop() {
    cpu_pool_.SoftStop();

    background_.stop();

    detail::HandlersProcessor<State>::stop();
    detail::WorkersProcessor<State>::stop();

    NetworkProcessor::stop();

    network_.stop();
    transport_.stop();

    cpu_pool_.Wait();
  }
}// namespace maelstrom