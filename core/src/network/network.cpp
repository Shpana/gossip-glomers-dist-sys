#include "network/network.hpp"

#include <thread>
#include <unordered_map>

#include <fmt/format.h>
#include <yaclib/async/contract.hpp>

#include "logging.hpp"
#include "network/messages.hpp"

using namespace std::chrono_literals;

namespace ds::core {
  namespace {
    constexpr Network::Clock::duration repeat_threshold{1s};
  }

  Network::Session::Session(Network& network, std::string source)
      : network_{network}, source_{std::move(source)} {}

  Network::Network(yaclib::IExecutorPtr executor,
                   std::shared_ptr<Transport> transport)
      : executor_{std::move(executor)}, transport_{std::move(transport)} {}

  void Network::start() {
    is_running_.store(true);

    timer_thread_ = std::thread{[this]() {
      updateWaiters();
    }};
  }

  void Network::stop() {
    is_running_.store(false);

    if (timer_thread_.joinable()) {
      timer_thread_.join();
    }
  }

  void Network::handle(Response&& response) {
    if (handleWaiters(waiters_once_, response)) {
      return;
    }
    if (handleWaiters(waiters_at_least_once_, response)) {
      return;
    }
  }

  Network::Session Network::makeSession(std::string source) {
    return Session{*this, std::move(source)};
  }

  void Network::send(Request&& request) {
    transport_->send(std::move(request).toMessage());
  }

  void Network::sendAtLeastOnce(Request&& request) {
    auto id = request.message_id;
    auto request_copy = request;
    transport_->send(std::move(request).toMessage());

    auto [_, p] = yaclib::MakeContract<Response>();

    auto now = Clock::now();

    {
      std::lock_guard guard{mtx_};
      waiters_at_least_once_.emplace(
          id,
          Waiter<Policy::AtLeastOnce>{.p = std::move(p),
                                      .last_updated = now,
                                      .request_copy = std::move(request_copy)});
    }
  }

  yaclib::FutureOn<Response> Network::call(Request&& request) {
    auto id = request.message_id;
    transport_->send(std::move(request).toMessage());

    auto [f, p] = yaclib::MakeContractOn<Response>(*executor_);

    {
      std::lock_guard guard{mtx_};
      waiters_once_.emplace(id, Waiter<Policy::Once>{.p = std::move(p)});
    }

    return std::move(f);
  }

  void Network::updateWaiters() {
    while (is_running_.load()) {
      auto now = Clock::now();

      {
        // TODO(shpana): is locking ok here?
        std::lock_guard guard{mtx_};

        for (auto& [id, waiter]: waiters_at_least_once_) {
          if (waiter.last_updated + repeat_threshold < now) {
            LOG_INFO() << "Repeating request!\n";
            auto request = waiter.request_copy;
            auto id = request.message_id;
            transport_->send(std::move(request).toMessage());
            waiter.last_updated = now;
          }
        }
      }

      // TODO(shpana): handle deadlines more precisely
      std::this_thread::sleep_for(repeat_threshold);
    }
  }
}// namespace ds::core