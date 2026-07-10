#pragma once

#include <optional>

#include <nlohmann/json.hpp>

namespace ds::core {
  struct Response {
    std::string sender;
    std::string recipient;
    std::string type;
    nlohmann::json body;
    std::uint64_t in_reply_to;
  };

  struct Request {
    std::string sender;
    std::string recipient;
    std::string type;
    nlohmann::json body;
    std::uint64_t message_id;

    static std::optional<Request> parse(std::string raw_raquest);

    [[nodiscard]] Response reply(std::string type, nlohmann::json body) const;
    [[nodiscard]] Response reply(std::string type) const;
    [[nodiscard]] Response reply(std::string_view type,
                                 nlohmann::json body) const;
    [[nodiscard]] Response reply(std::string_view type) const;
  };
}// namespace ds::core