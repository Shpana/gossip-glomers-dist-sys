#pragma once

#include <chrono>
#include <concepts>

#include <yaclib/async/future.hpp>

#include "environment.hpp"
#include "network/network.hpp"

namespace maelstrom {
  namespace detail {
    template<typename State>
    class WorkersProcessor;
  }

  template<typename State>
  class WorkerBase {
  public:
    using Clock = std::chrono::steady_clock;

  public:
    explicit WorkerBase(Clock::duration period) : period_{period} {}

    virtual ~WorkerBase() = default;

    virtual void start() { next_deadline_ = Clock::now() + period_; }
    virtual yaclib::Future<> process(Network::Session&& session) = 0;
    virtual void stop() {}

  protected:
    std::optional<Environment> env_{std::nullopt};
    std::shared_ptr<State> state_;

  private:
    Clock::duration period_;
    std::atomic<Clock::time_point> next_deadline_{};

    enum struct ExecutionState { Idle = 0, InProgress };

    std::atomic<ExecutionState> exec_state_{ExecutionState::Idle};

  private:
    friend detail::WorkersProcessor<State>;

    void startInternal(Environment env, std::shared_ptr<State> state);
    void stopInternal();
  };

  template<typename State>
  void WorkerBase<State>::startInternal(Environment env,
                                        std::shared_ptr<State> state) {
    env_.emplace(std::move(env));
    state_ = std::move(state);
  }

  template<typename State>
  void WorkerBase<State>::stopInternal() {}

  template<typename Worker, typename State>
  concept IsWorker =
      std::derived_from<Worker, WorkerBase<State>> && requires(Worker worker) {
        { Worker::type };
      };
}// namespace maelstrom