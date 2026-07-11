#pragma once

#include <unordered_map>

#include <yaclib/async/future.hpp>
#include <yaclib/async/promise.hpp>
#include <yaclib/exe/executor.hpp>

#include "messages.hpp"
#include "transport.hpp"

namespace ds::core {
  class Network {
  public:
    class Session;

  public:
    Network(yaclib::IExecutorPtr executor,
            std::shared_ptr<Transport> transport);

    void handle(Response&& response);
    [[nodiscard]] Session makeSession(std::string source);

  private:
    void send(Request&& request);
    yaclib::FutureOn<Response> call(Request&& request);

  private:
    yaclib::IExecutorPtr executor_;
    std::shared_ptr<Transport> transport_;

    std::uint64_t previous_id_{0};
    std::unordered_map<std::uint64_t, yaclib::Promise<Response>> waiters_{};
    std::mutex mtx_{};
  };

  class Network::Session {
  public:
    Session(Network& network, std::uint64_t id, std::string sourse);

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
    yaclib::FutureOn<Response> call(std::string destination,
                                    nlohmann::json body);

  private:
    Network& network_;
    std::uint64_t id_;
    std::string source_;
  };

  template<typename Handler>
  void Network::Session::send(std::string destination, nlohmann::json body) {
    network_.send(Request{.source = source_,
                          .destination = std::move(destination),
                          .type = std::string{Handler::type},
                          .body = std::move(body),
                          .message_id = id_});
  }

  template<typename Handler>
  yaclib::FutureOn<Response>
  Network::Session::Session::call(std::string destination,
                                  nlohmann::json body) {
    return network_.call(Request{.source = source_,
                                 .destination = std::move(destination),
                                 .type = std::string{Handler::type},
                                 .body = std::move(body),
                                 .message_id = id_});
  }
}// namespace ds::core