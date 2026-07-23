#include <maelstrom/detail/processors/network.hpp>

#include <fmt/format.h>
#include <yaclib/async/contract.hpp>
#include <yaclib/util/result.hpp>

#include <maelstrom/network/messages.hpp>

namespace maelstrom::detail {

namespace {
using namespace std::chrono_literals;
constexpr NetworkProcessor::Clock::duration repeat_threshold{500ms};
} // namespace

void NetworkProcessor::start() {
  std::exchange(is_running_, true);

  assistant_ = std::thread{[this]() { backgroundUpdate(); }};
}

void NetworkProcessor::stop() {
  std::exchange(is_running_, false);

  if (assistant_.joinable()) {
    assistant_.join();
  }
}

void NetworkProcessor::process(Response response) {
  if (processInternal(once_, response)) {
    return;
  }
  if (processInternal(at_least_once_, response)) {
    return;
  }
}

void NetworkProcessor::useTransport(std::shared_ptr<ITransport> transport) {
  transport_ = std::move(transport);
}

template <WaitPolicy P>
bool NetworkProcessor::processInternal(Waiters<P> &waiters,
                                       Response &response) {
  yaclib::Promise<Response> p;

  {
    std::lock_guard guard{mtx_};

    auto it = waiters.find(response.in_reply_to);
    if (it == waiters.end()) {
      return false;
    }

    p = std::move(it->second.p);
    waiters.erase(it);
  }

  std::move(p).Set(std::move(response));
  return true;
}

void NetworkProcessor::send(Request request) {
  callDetachedInternal(std::move(request));
}

void NetworkProcessor::sendAtLeastOnce(Request request,
                                       std::optional<Clock::duration> timeout) {
  std::ignore = callAtLeastOnceInternal(std::move(request), timeout);
}

yaclib::Future<Response>
NetworkProcessor::call(Request request,
                       std::optional<Clock::duration> timeout) {
  return callOnceInternal(std::move(request), timeout);
}

yaclib::Future<Response>
NetworkProcessor::callAtLeastOnce(Request request,
                                  std::optional<Clock::duration> timeout) {
  return callAtLeastOnceInternal(std::move(request), timeout);
}

void NetworkProcessor::callDetachedInternal(Request request) {
  transport_->send(std::move(request).toMessage());
}

yaclib::Future<Response>
NetworkProcessor::callOnceInternal(Request request,
                                   std::optional<Clock::duration> timeout) {
  auto id = request.message_id;
  transport_->send(std::move(request).toMessage());

  auto [f, p] = yaclib::MakeContract<Response>();

  std::optional<Clock::time_point> deadline = std::nullopt;
  if (timeout.has_value()) {
    deadline.emplace(Clock::now() + timeout.value());
  }

  {
    std::lock_guard guard{mtx_};

    once_.emplace(
        id, Waiter<WaitPolicy::Once>{.p = std::move(p), .deadline = deadline});
  }

  return std::move(f);
}

yaclib::Future<Response> NetworkProcessor::callAtLeastOnceInternal(
    Request request, std::optional<Clock::duration> timeout) {
  auto id = request.message_id;
  auto request_copy = request;
  transport_->send(std::move(request).toMessage());

  auto [f, p] = yaclib::MakeContract<Response>();

  auto now = Clock::now();

  std::optional<Clock::time_point> deadline = std::nullopt;
  if (timeout.has_value()) {
    deadline.emplace(now + timeout.value());
  }

  {
    std::lock_guard guard{mtx_};
    at_least_once_.emplace(id, Waiter<WaitPolicy::AtLeastOnce>{
                                   .p = std::move(p),
                                   .updated_at = now,
                                   .deadline = deadline,
                                   .request_copy = std::move(request_copy)});
  }

  return std::move(f);
}

void NetworkProcessor::backgroundUpdate() {
  while (is_running_) {
    updateOnce();
    updateAtLeastOnce();
    // TODO(shpana): handle deadlines more precisely
    std::this_thread::sleep_for(repeat_threshold);
  }
}

void NetworkProcessor::updateOnce() {
  auto now = Clock::now();

  {
    std::lock_guard guard{mtx_};
    eraseExpiredDeadlines(once_, now);
  }
}

void NetworkProcessor::updateAtLeastOnce() {
  auto now = Clock::now();

  {
    std::lock_guard guard{mtx_};

    eraseExpiredDeadlines(at_least_once_, now);

    for (auto &[id, waiter] : at_least_once_) {
      if (waiter.updated_at + repeat_threshold < now) {
        auto request = waiter.request_copy;
        auto id = request.message_id;
        transport_->send(std::move(request).toMessage());

        waiter.updated_at = now;
      }
    }
  }
}

template <WaitPolicy P>
void NetworkProcessor::eraseExpiredDeadlines(Waiters<P> &waiters,
                                             Clock::time_point now) {
  for (auto &[id, waiter] : waiters) {
    if (waiter.deadline.has_value() && waiter.deadline.value() < now) {
      std::move(waiter.p).Set(Error{.source = "unknown",
                                    .destination = "unknown",
                                    .code = ErrorCode::Timeout,
                                    .what = "Timeout",
                                    .body = nlohmann::json({}),
                                    .in_reply_to = id}
                                  .toResponse());
    }
  }

  std::erase_if(waiters, [now](const auto &elem) {
    auto &deadline = elem.second.deadline;
    return deadline.has_value() && deadline.value() < now;
  });
}

} // namespace maelstrom::detail