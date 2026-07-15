#pragma once

#include <yaclib/async/future.hpp>
#include <yaclib/async/promise.hpp>
#include <yaclib/exe/executor.hpp>

#include "network/messages.hpp"
#include "network/transport.hpp"

namespace ds::core::detail {
  enum struct WaitPolicy { Detached = 0, Once, AtLeastOnce };

  class NetworkProcessor {
  public:
    using Clock = std::chrono::steady_clock;

  private:
    template<WaitPolicy P>
    struct Waiter;

    template<WaitPolicy P>
    using Waiters = std::unordered_map<std::uint64_t, Waiter<P>>;

    template<>
    struct Waiter<WaitPolicy::Detached> {};

    template<>
    struct Waiter<WaitPolicy::Once> {
      yaclib::Promise<Response> p;
      std::optional<Clock::time_point> deadline;
    };

    template<>
    struct Waiter<WaitPolicy::AtLeastOnce> {
      yaclib::Promise<Response> p;
      Clock::time_point updated_at;
      std::optional<Clock::time_point> deadline;
      Request request_copy;
    };

  public:
    explicit NetworkProcessor(Transport& transport);

    void start();
    void stop();

    void process(Response&& response);

    void send(Request&& request);
    void sendAtLeastOnce(Request&& request,
                         std::optional<Clock::duration> timeout = std::nullopt);
    yaclib::Future<Response>
    call(Request&& request,
         std::optional<Clock::duration> timeout = std::nullopt);
    yaclib::Future<Response>
    callAtLeastOnce(Request&& request,
                    std::optional<Clock::duration> timeout = std::nullopt);

  private:
    template<WaitPolicy P>
    bool processInternal(Waiters<P>& waiters, Response& response);

    // TODO(shpana): use cancellation tokens?

    void callDetachedInternal(Request&& request);
    yaclib::Future<Response>
    callOnceInternal(Request&& request, std::optional<Clock::duration> timeout);
    // TODO(shpana): make exponential backoffs strategy
    yaclib::Future<Response>
    callAtLeastOnceInternal(Request&& request,
                            std::optional<Clock::duration> timeout);

    void backgroundUpdate();

    void updateOnce();
    void updateAtLeastOnce();

    // Not thead-safe
    template<WaitPolicy P>
    void eraseExpiredDeadlines(Waiters<P>& waiters, Clock::time_point now);

  private:
    bool is_running_{false};

    Transport& transport_;

    std::mutex mtx_{};
    Waiters<WaitPolicy::Once> once_{};                // Guarded by mtx_
    Waiters<WaitPolicy::AtLeastOnce> at_least_once_{};// Guarded by mtx_
  };

}// namespace ds::core::detail