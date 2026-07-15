#pragma once

#include <exception>

// TODO(shpana): why here?

namespace maelstrom::detail {
  struct TimeoutException : public std::exception {};
}// namespace maelstrom::detail