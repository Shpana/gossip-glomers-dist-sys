#pragma once

#include <functional>

#include <yaclib/async/future.hpp>

namespace ds::core::detail {
  using Action = std::function<void()>;
}// namespace ds::core::detail