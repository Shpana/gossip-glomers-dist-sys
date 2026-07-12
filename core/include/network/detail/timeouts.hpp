#pragma once

#include <exception>

namespace ds::core::detail {
  struct TimeoutException : public std::exception {};
}// namespace ds::core::detail