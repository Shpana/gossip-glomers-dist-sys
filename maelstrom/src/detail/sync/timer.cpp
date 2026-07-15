#include "detail/sync/timer.hpp"

#include "log/logging.hpp"

namespace maelstrom::detail {
  Timer::Timer(yaclib::IExecutor& executor) : executor_{executor} {}

  void Timer::start() {
    is_running_.store(true);

    assistant_ = std::thread{[this] {
      while (is_running_) {
        process();
      }
    }};
  }

  void Timer::stop() {
    is_running_.store(false);

    is_empty_.notify_all();
    process_on_time_.notify_all();

    if (assistant_.joinable()) {
      assistant_.join();
    }
  }

  void Timer::submit(Action&& action) {
    std::ignore = yaclib::Run(executor_, std::move(action));
  }

  void Timer::set(Clock::duration delay, Action&& action) {
    if (!is_running_) {
      return;
    }

    auto now = Clock::now();
    set(now + delay, std::move(action));
  }

  void Timer::set(Clock::time_point deadline, Action&& action) {
    if (!is_running_) {
      return;
    }

    {
      std::unique_lock lock{mtx_};

      waiters_.insert({deadline, std::move(action)});

      is_empty_.notify_one();
      process_on_time_.notify_one();
    }
  }

  void Timer::process() {
    {
      std::unique_lock lock{mtx_};
      while (is_running_.load() && waiters_.empty()) {
        is_empty_.wait(lock);
      }

      if (!waiters_.empty()) {
        auto [deadline, action] = *waiters_.begin();
        waiters_.erase(waiters_.begin());

        auto now = Clock::now();
        if (deadline < now) {
          submit(std::move(action));
          return;
        }

        process_on_time_.wait_until(lock, deadline);

        waiters_.insert({deadline, std::move(action)});
      }
    }
  }
}// namespace maelstrom::detail