#pragma once

#include <chrono>
#include <concepts>

#include <yaclib/async/future.hpp>

#include "environment.hpp"
#include "network/network.hpp"
#include "utils/unit.hpp"

namespace ds::core {
  template<typename State>
  class Node;

  // Processes background tasks
  template<typename State>
  class ServiceBase {
  public:
    using Clock = std::chrono::steady_clock;

  public:
    explicit ServiceBase(Clock::duration period) : period_{period} {}

    virtual ~ServiceBase() = default;

    virtual void start() {}
    virtual yaclib::Future<core::Unit> process(Network::Session&& session) = 0;
    virtual void stop() {}

  protected:
    std::optional<Environment<State>> env_{std::nullopt};

  private:
    Clock::duration period_;
    Clock::time_point processed_at_{};

    enum struct ExecutionState { Idle = 0, InProgress };

    std::atomic<ExecutionState> state_{ExecutionState::Idle};

  private:
    friend Node<State>;

    void startInternal(Environment<State> env) { env_.emplace(std::move(env)); }
    void stopInternal() {}
  };

  template<typename Service, typename State>
  concept IsService = std::derived_from<Service, ServiceBase<State>> &&
                      requires(Service service) {
                        { Service::type };
                      };
}// namespace ds::core