#pragma once

#include <condition_variable>
#include <deque>
#include <mutex>
#include <optional>

namespace maelstrom::detail {
  template<typename V>
  class UnboundedBlockingQueue {
  public:
    void push(V value) {
      std::lock_guard guard{mtx_};
      if (!is_closed_) {
        buffer_.push_back(std::move(value));
        not_empty_.notify_one();
      }
    }

    std::optional<V> pop() {
      std::unique_lock lock{mtx_};
      while (buffer_.empty()) {
        if (is_closed_) {
          return std::nullopt;
        }
        not_empty_.wait(lock);
      }
      return std::make_optional<V>(popLocked());
    }

    [[nodiscard]] bool isEmpty() const {
      std::lock_guard guard{mtx_};
      return buffer_.empty();
    }

    [[nodiscard]] std::size_t size() const {
      std::lock_guard guard{mtx_};
      return buffer_.size();
    }

    void close() {
      std::lock_guard guard{mtx_};
      is_closed_ = true;
      not_empty_.notify_all();
    }

    [[nodiscard]] bool isClosed() const {
      std::lock_guard guard{mtx_};
      return is_closed_;
    }

  private:
    V popLocked() {
      auto result = std::move(buffer_.front());
      buffer_.pop_front();
      return result;
    }

  private:
    mutable std::mutex mtx_{};

    bool is_closed_{false};              // Guarded by mtx_
    std::deque<V> buffer_{};             // Guarded by mtx_
    std::condition_variable not_empty_{};// Guarded by mtx_
  };
}// namespace maelstrom::detail