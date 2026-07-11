#pragma once

#include <chrono>
#include <memory>
#include <unordered_map>

#include <yaclib/async/future.hpp>
#include <yaclib/async/promise.hpp>
#include <yaclib/exe/executor.hpp>

#include "messages.hpp"
#include "transport.hpp"

namespace ds::core {
  class Network {
    enum struct Policy { Once = 0, AtLeastOnce };

    template<Policy P>
    struct Waiter;

  public:
    class Session;

    using Clock = std::chrono::steady_clock;

  public:
    Network(yaclib::IExecutorPtr executor,
            std::shared_ptr<Transport> transport);

    void start();
    void stop();

    void handle(Response&& response);
    [[nodiscard]] Session makeSession(std::string source);

  private:
    template<typename Waiter>
    bool handleWaiters(std::unordered_map<std::uint64_t, Waiter>& waiters,
                       Response& response) {
      auto it = waiters.find(response.in_reply_to);

      if (it == waiters.end()) {
        return false;
      }

      yaclib::Promise<Response> p;

      {
        std::lock_guard guard{mtx_};
        p = std::move(it->second.p);
        waiters.erase(it);
      }

      std::move(p).Set(std::move(response));
      return true;
    }

    void send(Request&& request);
    void sendAtLeastOnce(Request&& request);
    yaclib::FutureOn<Response> call(Request&& request);

    void updateWaiters();

  private:
    yaclib::IExecutorPtr executor_;
    std::shared_ptr<Transport> transport_;

    std::thread timer_thread_;

    std::atomic<bool> is_running_{false};

    std::mutex mtx_{};
    std::atomic<std::uint64_t> previous_id_{0};
    std::unordered_map<std::uint64_t, Waiter<Policy::Once>>
        waiters_once_{};// Guarded by mtx_
    std::unordered_map<std::uint64_t, Waiter<Policy::AtLeastOnce>>
        waiters_at_least_once_{};// Guarded by mtx_
  };

  template<>
  struct Network::Waiter<Network::Policy::Once> {
    yaclib::Promise<Response> p;
  };

  template<>
  struct Network::Waiter<Network::Policy::AtLeastOnce> {
    yaclib::Promise<Response> p;
    Clock::time_point last_updated;
    Request request_copy;
  };

  class Network::Session {
  public:
    Session(Network& network, std::string sourse);

    // Non-copyable
    Session(const Session&) = delete;
    Session& operator=(const Session&) = delete;
    // Movable
    Session(Session&&) = default;
    // Non move-assignable
    Session& operator=(Session&&) = delete;

    ~Session() = default;

    template<typename Handler>
    void send(std::string destination, nlohmann::json body);
    template<typename Handler>
    void sendAtLeastOnce(std::string destination, nlohmann::json body);
    template<typename Handler>
    yaclib::FutureOn<Response> call(std::string destination,
                                    nlohmann::json body);

  private:
    template<typename Handler>
    [[nodiscard]] Request makeRequest(std::string destination,
                                      nlohmann::json body) const;

  private:
    Network& network_;
    std::string source_;
  };

  template<typename Handler>
  void Network::Session::send(std::string destination, nlohmann::json body) {
    network_.send(
        makeRequest<Handler>(std::move(destination), std::move(body)));
  }

  template<typename Handler>
  void Network::Session::sendAtLeastOnce(std::string destination,
                                         nlohmann::json body) {
    network_.sendAtLeastOnce(
        makeRequest<Handler>(std::move(destination), std::move(body)));
  }

  template<typename Handler>
  yaclib::FutureOn<Response>
  Network::Session::Session::call(std::string destination,
                                  nlohmann::json body) {
    return network_.call(
        makeRequest<Handler>(std::move(destination), std::move(body)));
  }

  template<typename Handler>
  Request Network::Session::makeRequest(std::string destination,
                                        nlohmann::json body) const {
    return Request{.source = source_,
                   .destination = std::move(destination),
                   .type = std::string{Handler::type},
                   .body = std::move(body),
                   .message_id = network_.previous_id_.fetch_add(1)};
  }
}// namespace ds::core