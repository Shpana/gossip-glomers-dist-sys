#pragma once

#include <cstdint>
#include <string_view>

// TODO(shpana): why here?

namespace maelstrom {
  constexpr std::string_view error_type = "error";

  enum struct ErrorCode : std::uint8_t {
    Timeout = 0,
    NodeNotFound = 1,
    NotSupported = 10,
    TemporarilyUnavailable = 11,
    MalformedRequest = 12,
    Crash = 13,
    Abort = 14,
    KeyDoesNotExists = 20,
    KeyAlreadyExists = 21,
    PreconditionFailed = 22,
    TxnConflict = 30
  };
}//namespace maelstrom