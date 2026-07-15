#pragma once

#include <memory>
#include <string>
#include <thread>
#include <unordered_map>

#include <yaclib/async/run.hpp>
#include <yaclib/coro/future.hpp>
#include <yaclib/exe/executor.hpp>

#include "detail/sync/timer.hpp"
#include "environment.hpp"
#include "network/transport.hpp"
#include "routines/worker.hpp"

namespace maelstrom::detail {
  template<typename State>
  class WorkersProcessor {
  public:
    WorkersProcessor(Timer& timer, Network& network);

    template<IsWorker<State> Handler, typename... Args>
    void add(Args&&... args);

    void start(Environment env, std::shared_ptr<State> state);
    void stop();

  private:
    void process();

  private:
    bool is_running_{false};

    std::thread assistant_;

    Timer& timer_;

    Network& network_;

    std::unordered_map<std::string, std::unique_ptr<WorkerBase<State>>>
        workers_{};
  };

  template<typename State>
  WorkersProcessor<State>::WorkersProcessor(Timer& timer, Network& network)
      : timer_{timer}, network_{network} {}

  template<typename State>
  template<IsWorker<State> Worker, typename... Args>
  void WorkersProcessor<State>::add(Args&&... args) {
    if (is_running_) {
      LOG_ERROR() << fmt::format(
          "Cannot add worker '{}', node already started!\n", Worker::type);
      return;
    }

    auto worker = std::make_unique<Worker>(std::forward<Args>(args)...);
    workers_[std::string{Worker::type}] = std::move(worker);
    LOG_INFO() << fmt::format("Worker '{}' was added to node registry!\n",
                              Worker::type);
  }

  template<typename State>
  void WorkersProcessor<State>::start(Environment env,
                                      std::shared_ptr<State> state) {
    std::exchange(is_running_, true);

    for (auto& [type, worker]: workers_) {
      worker->startInternal(env, state);
      worker->start();
    }

    timer_.submit([this]() { process(); });
  }

  template<typename State>
  void WorkersProcessor<State>::stop() {
    if (is_running_) {
      std::exchange(is_running_, false);

      for (auto& [type, worker]: workers_) {
        worker->stop();
        worker->stopInternal();
      }
    }
  }

  template<typename State>
  void WorkersProcessor<State>::process() {
    using Clock = WorkerBase<State>::Clock;
    using namespace std::chrono_literals;

    using ExecutionState = WorkerBase<State>::ExecutionState;

    if (!is_running_) {
      return;
    }

    auto now = Clock::now();

    constexpr typename Clock::duration max_backoff{10s};
    typename Clock::time_point next_deadline = now + max_backoff;

    for (auto& [type, worker]: workers_) {
      next_deadline = std::min(next_deadline, now + worker->period_);

      auto guess = ExecutionState::Idle;
      if (!worker->exec_state_.compare_exchange_strong(
              guess, ExecutionState::InProgress)) {
        continue;
      }

      timer_.set(
          worker->next_deadline_,
          [this, type, &worker]() mutable -> yaclib::Future<> {
            try {
              auto session = network_.makeSession();
              co_await worker->process(std::move(session));
            } catch (const std::exception& ex) {
              LOG_ERROR() << fmt::format(
                  "Exception occurs, when processing worker '{}', ex: {}\n",
                  type, ex.what());
            }

            worker->next_deadline_ = Clock::now() + worker->period_;
            worker->exec_state_.store(ExecutionState::Idle);
          });
    }

    timer_.set(next_deadline, [this]() { process(); });
  }
}// namespace maelstrom::detail