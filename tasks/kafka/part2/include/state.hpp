#pragma once

#include <cstdint>
#include <map>
#include <string>

namespace tasks::kafka::part2 {
  using Offset = std::uint64_t;
  using Message = std::uint64_t;

  using Log = std::map<Offset, Message>;
  using Committed = std::map<std::string, Offset>;

  struct State {};
}// namespace tasks::kafka::part2