#pragma once

#include <chrono>
#include <concepts>

#include <yaclib/async/future.hpp>

#include <maelstrom/environment.hpp>
#include <maelstrom/log/logging.hpp>
#include <maelstrom/network/network.hpp>

namespace maelstrom {

namespace detail {
template <typename State> class WorkersProcessor;
}

template <typename State> class WorkerBase {
public:
  using Clock = std::chrono::steady_clock;

public:
  explicit WorkerBase(Clock::duration period);
  virtual ~WorkerBase() = default;

  virtual void start();
  virtual void stop();

  virtual yaclib::Future<> process(Network::Session session) = 0;

protected:
  [[nodiscard]] std::shared_ptr<State> getState() const;
  [[nodiscard]] const Environment &getEnvironment() const;

private:
  friend detail::WorkersProcessor<State>;

  void startInternal(Environment env, std::shared_ptr<State> state);
  void stopInternal();

private:
  std::optional<Environment> env_{std::nullopt};
  std::shared_ptr<State> state_;

private:
  // TODO(shpana): make private inheretence
  // from intrusive base for scheduling in network
  Clock::duration period_;
  // TODO(shpana): initialization of next_deadline_ should occurs on start
  std::atomic<Clock::time_point> next_deadline_{};

  enum struct ExecutionState { Idle = 0, InProgress };

  std::atomic<ExecutionState> exec_state_{ExecutionState::Idle};
};

template <typename Worker, typename State>
concept IsWorker =
    std::derived_from<Worker, WorkerBase<State>> && requires(Worker worker) {
      { Worker::type };
    };

} // namespace maelstrom

template <typename State>
maelstrom::WorkerBase<State>::WorkerBase(Clock::duration period)
    : period_{period} {}

template <typename State> void maelstrom::WorkerBase<State>::start() {}

template <typename State> void maelstrom::WorkerBase<State>::stop() {}

template <typename State>
void maelstrom::WorkerBase<State>::startInternal(Environment env,
                                                 std::shared_ptr<State> state) {
  env_.emplace(std::move(env));
  state_ = std::move(state);
  next_deadline_ = Clock::now() + period_;
}

template <typename State> void maelstrom::WorkerBase<State>::stopInternal() {}

template <typename State>
std::shared_ptr<State> maelstrom::WorkerBase<State>::getState() const {
  if (!state_) [[unlikely]] {
    LOG_ERROR() << "Cannot get state for uninitialized node!\n";
    throw std::runtime_error{"Cannot get state for uninitialized node!"};
  }

  return state_;
}

template <typename State>
const maelstrom::Environment &
maelstrom::WorkerBase<State>::getEnvironment() const {
  if (!env_.has_value()) [[unlikely]] {
    LOG_ERROR() << "Cannot get environment for uninitialized node!\n";
    throw std::runtime_error{"Cannot get environment for uninitialized node!"};
  }

  return env_.value();
}