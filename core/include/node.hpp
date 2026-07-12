#pragma once

#include <exception>
#include <memory>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

#include <yaclib/async/future.hpp>
#include <yaclib/async/run.hpp>
#include <yaclib/runtime/fair_thread_pool.hpp>
#include <yaclib/util/intrusive_ptr.hpp>

#include "environment.hpp"
#include "logging.hpp"
#include "network/messages.hpp"
#include "network/network.hpp"
#include "network/transport.hpp"
#include "routines/handler.hpp"
#include "routines/service.hpp"

namespace ds::core {
  template<typename State>
  class Node {
  public:
    Node();

    void run();

    template<IsHandler<State> Handler, typename... Args>
    void add(Args&&... args);
    template<IsService<State> Service, typename... Args>
    void add(Args&&... args);

  private:
    bool start();
    bool loadEnvironment();

    void process();
    void scheduleProcessing(std::string type, ServiceBase<State>& service);

    void handle(Message&& message);
    void handle(Request&& request);
    void handle(Response&& response);

    void stop();

    [[nodiscard]] bool shouldRun() const;

  private:
    yaclib::IntrusivePtr<yaclib::FairThreadPool> thread_pool_;
    std::thread assistant_;

    std::shared_ptr<Transport> transport_;
    std::optional<Environment<State>> env_{std::nullopt};

    std::shared_ptr<Network> network_;

    bool was_started_{false};
    std::unordered_map<std::string, std::unique_ptr<HandlerBase<State>>>
        handlers_{};
    std::unordered_map<std::string, std::unique_ptr<ServiceBase<State>>>
        services_{};
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

      assistant_ = std::thread{[this]() {
        process();
      }};

      while (shouldRun()) {
        auto message = transport_->recieve();
        if (!message.has_value()) {
          continue;
        }

        handle(std::move(message.value()));
      }
    }

    stop();
  }

  template<typename State>
  template<IsHandler<State> Handler, typename... Args>
  void Node<State>::add(Args&&... args) {
    if (was_started_) {
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
  template<IsService<State> Service, typename... Args>
  void Node<State>::add(Args&&... args) {
    if (was_started_) {
      LOG_ERROR() << fmt::format(
          "Cannot add service '{}', node already started!\n", Service::type);
      return;
    }

    auto service = std::make_unique<Service>(std::forward<Args>(args)...);
    services_[std::string{Service::type}] = std::move(service);
    LOG_INFO() << fmt::format("Service '{}' was added to node registry!\n",
                              Service::type);
  }

  template<typename State>
  bool Node<State>::start() {
    transport_->start();
    network_->start();

    if (!loadEnvironment()) {
      return false;
    }

    for (auto& [type, handler]: handlers_) {
      handler->startInternal(env_.value());
      handler->start();
    }
    for (auto& [type, service]: services_) {
      service->startInternal(env_.value());
      service->start();
    }

    std::exchange(was_started_, true);
    return true;
  }

  template<typename State>
  bool Node<State>::loadEnvironment() {
    auto message = transport_->recieve();
    if (!message.has_value()) {
      LOG_ERROR() << "Failed to start node: cannot parse message!\n";
      return false;
    }

    constexpr std::string_view init_type = "init";

    auto init_request = std::move(message.value()).toRequest();
    if (!init_request.has_value() || init_request.value().type != init_type) {
      LOG_ERROR() << "Failed to start node: errors in init's request!\n";
      return false;
    }

    const auto& body = init_request.value().body;
    env_.emplace(Environment<State>{
        .node_id = body["node_id"].get<std::string>(),
        .available_node_ids = body["node_ids"].get<std::vector<std::string>>(),
        .state = std::make_shared<State>()});

    transport_->send(std::move(init_request.value()).toResponse().toMessage());
    return true;
  }

  template<typename State>
  void Node<State>::process() {
    using TimePoint = ServiceBase<State>::Clock::time_point;
    using Duration = ServiceBase<State>::Clock::duration;
    using namespace std::chrono_literals;

    constexpr Duration max_deadline_duration{2s};
    TimePoint next_deadline;

    for (auto& [type, service]: services_) {
      scheduleProcessing(type, *service);
      auto now = ServiceBase<State>::Clock::now();
      next_deadline = std::min(next_deadline, now + service->period_);
    }

    while (shouldRun()) {
      auto now = ServiceBase<State>::Clock::now();
      next_deadline = std::min(next_deadline, now + max_deadline_duration);

      std::this_thread::sleep_until(next_deadline);

      using ExecutionState = ServiceBase<State>::ExecutionState;

      for (auto& [type, service]: services_) {
        auto now = ServiceBase<State>::Clock::now();
        if (service->state_.load() == ExecutionState::Idle &&
            service->processed_at_ + service->period_ < now) {
          scheduleProcessing(type, *service);
        }
        next_deadline = std::min(next_deadline, now + service->period_);
      }
    }
  }

  template<typename State>
  void Node<State>::scheduleProcessing(std::string type,
                                       ServiceBase<State>& service) {
    using ExecutionState = ServiceBase<State>::ExecutionState;

    assert(service.state_.load() == ExecutionState::Idle);
    service.state_.store(ExecutionState::InProgress);

    auto now = ServiceBase<State>::Clock::now();
    // Access to proccessed_at_ serialized by state_
    service.processed_at_ = now;

    auto f = yaclib::Run(*thread_pool_, [this, type = std::move(type),
                                         &service]() {
      try {
        auto session = network_->makeSession(env_->node_id);
        auto unit = service.process(std::move(session)).Get().Ok();
      } catch (const std::exception& ex) {
        LOG_ERROR() << fmt::format(
            "There is error, when processing '{}', ex: {}\n", type, ex.what());
      }

      service.state_.store(ExecutionState::Idle);
    });
  }

  template<typename State>
  void Node<State>::handle(Message&& message) {
    if (message.isRequest()) {
      return handle(std::move(message).toRequest().value());
    } else if (message.isResponse()) {
      return handle(std::move(message).toResponse().value());
    }
    LOG_ERROR() << "Unknown type of message!\n";
  }

  template<typename State>
  void Node<State>::handle(Request&& request) {
    const auto& type = request.type;

    auto it = handlers_.find(type);
    if (it == handlers_.end()) {
      LOG_ERROR() << fmt::format("No handler for message of type '{}'\n", type);
      return;
    }

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
      for (auto& [type, service]: services_) {
        service->stop();
        service->stopInternal();
      }

      if (assistant_.joinable()) {
        assistant_.join();
      }

      thread_pool_->SoftStop();
      thread_pool_->Wait();
    }

    network_->stop();
    transport_->stop();
  }

  template<typename State>
  bool Node<State>::shouldRun() const {
    return transport_->isStreaming();
  }
}// namespace ds::core