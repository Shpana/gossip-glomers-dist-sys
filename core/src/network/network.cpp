#include "network/network.hpp"

#include <fmt/format.h>
#include <yaclib/async/contract.hpp>

#include "logging.hpp"
#include "network/messages.hpp"

namespace ds::core {
  Network::Session::Session(Network& network, std::uint64_t id,
                            std::string source)
      : network_{network}, id_{id}, source_{std::move(source)} {}

  Network::Network(yaclib::IExecutorPtr executor,
                   std::shared_ptr<Transport> transport)
      : executor_{std::move(executor)}, transport_{std::move(transport)} {}

  void Network::handle(Response&& response) {
    auto id = response.in_reply_to;
    if (auto it = waiters_.find(id); it != waiters_.end()) {
      yaclib::Promise<Response> p;

      {
        std::lock_guard guard{mtx_};
        p = std::move(it->second);
        waiters_.erase(it);
      }

      std::move(p).Set(std::move(response));
    }

    LOG_ERROR() << fmt::format("No waiters for id={}!\n", id);
  }

  Network::Session Network::makeSession(std::string source) {
    return Session{*this, ++previous_id_, std::move(source)};
  }

  void Network::send(Request&& request) {
    transport_->send(std::move(request).toMessage());
  }

  yaclib::FutureOn<Response> Network::call(Request&& request) {
    auto id = request.message_id;
    transport_->send(std::move(request).toMessage());

    auto [f, p] = yaclib::MakeContractOn<Response>(*executor_);

    {
      std::lock_guard guard{mtx_};
      waiters_.emplace(id, std::move(p));
    }

    return std::move(f);
  }
}// namespace ds::core