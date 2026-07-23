#pragma once

#include <concepts>
#include <optional>

#include <yaclib/async/future.hpp>

#include <maelstrom/environment.hpp>
#include <maelstrom/log/logging.hpp>
#include <maelstrom/network/messages.hpp>
#include <maelstrom/network/network.hpp>

namespace maelstrom {

namespace detail {
template <typename State> class HandlersProcessor;
}

template <typename State> class HandlerBase {
public:
  virtual ~HandlerBase() = default;

  virtual void start();
  virtual void stop();

  virtual yaclib::Future<Response> handle(Network::Session session,
                                          Request request) = 0;

protected:
  [[nodiscard]] std::shared_ptr<State> getState() const;
  [[nodiscard]] const Environment &getEnvironment() const;

private:
  friend detail::HandlersProcessor<State>;

  void startInternal(Environment env, std::shared_ptr<State> state);
  void stopInternal();

private:
  std::optional<const Environment> env_{std::nullopt};
  std::shared_ptr<State> state_;
};

template <typename Handler, typename State>
concept IsHandler = std::derived_from<Handler, HandlerBase<State>> &&
                    requires(Handler handler) {
                      { Handler::type };
                    };

} // namespace maelstrom

template <typename State> void maelstrom::HandlerBase<State>::start() {}

template <typename State> void maelstrom::HandlerBase<State>::stop() {}

template <typename State>
void maelstrom::HandlerBase<State>::startInternal(
    Environment env, std::shared_ptr<State> state) {
  env_.emplace(std::move(env));
  state_ = std::move(state);
}

template <typename State> void maelstrom::HandlerBase<State>::stopInternal() {}

template <typename State>
std::shared_ptr<State> maelstrom::HandlerBase<State>::getState() const {
  if (!state_) [[unlikely]] {
    LOG_ERROR() << "Cannot get state for uninitialized node!\n";
    throw std::runtime_error{"Cannot get state for uninitialized node!"};
  }

  return state_;
}

template <typename State>
const maelstrom::Environment &
maelstrom::HandlerBase<State>::getEnvironment() const {
  if (!env_.has_value()) [[unlikely]] {
    LOG_ERROR() << "Cannot get environment for uninitialized node!\n";
    throw std::runtime_error{"Cannot get environment for uninitialized node!"};
  }

  return env_.value();
}