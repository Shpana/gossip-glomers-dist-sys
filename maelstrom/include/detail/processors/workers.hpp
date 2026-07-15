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
#include "routines/worker.hpp"

namespace maelstrom::detail {
  template<typename State>
  class WorkersProcessor {
  public:
    explicit WorkersProcessor(yaclib::IExecutor& executor, Network& network);

    template<IsWorker<State> Handler, typename... Args>
    void add(Args&&... args);

    void start(Environment env, std::shared_ptr<State> state);
    void stop();

  private:
    void backgroundProcess();

  private:
    bool is_running_{false};

    yaclib::IExecutor& executor_;
    std::thread assistant_;

    Network& network_;

    std::unordered_map<std::string, std::unique_ptr<WorkerBase<State>>>
        workers_{};
  };

  template<typename State>
  WorkersProcessor<State>::WorkersProcessor(yaclib::IExecutor& executor,
                                            Network& network)
      : executor_{executor}, network_{network} {}

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

    assistant_ = std::thread{[this]() {
      while (is_running_) {
        backgroundProcess();
      }
    }};
  }

  template<typename State>
  void WorkersProcessor<State>::stop() {
    if (is_running_) {
      std::exchange(is_running_, false);

      for (auto& [type, worker]: workers_) {
        worker->stop();
        worker->stopInternal();
      }

      if (assistant_.joinable()) {
        assistant_.join();
      }
    }
  }

  template<typename State>
  void WorkersProcessor<State>::backgroundProcess() {
    using Clock = WorkerBase<State>::Clock;
    using namespace std::chrono_literals;

    using ExecutionState = WorkerBase<State>::ExecutionState;

    if (!is_running_) {
      return;
    }

    auto now = Clock::now();

    constexpr typename Clock::duration max_backoff{1s};
    typename Clock::time_point next_deadline = now + max_backoff;

    for (auto& [type, worker]: workers_) {
      next_deadline = std::min(next_deadline, worker->next_deadline_.load());

      if (worker->next_deadline_.load() < now &&
          worker->exec_state_.load() == ExecutionState::Idle) {
        yaclib::Run(
            executor_, [this, type, &worker]() mutable -> yaclib::Future<> {
              try {
                auto session = network_.makeSession();
                co_await worker->process(std::move(session));
              } catch (const std::exception& ex) {
                LOG_ERROR() << fmt::format(
                    "Exception occurs, when processing worker '{}', ex: {}\n",
                    type, ex.what());
              }

              worker->next_deadline_.store(Clock::now() + worker->period_);
              worker->exec_state_.store(ExecutionState::Idle);
            });
      }
    }

    // TODO(shpana): wake up on stopping
    std::this_thread::sleep_until(next_deadline);
  }
}// namespace maelstrom::detail