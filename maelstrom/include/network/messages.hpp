#pragma once

#include <optional>

#include <nlohmann/json.hpp>

namespace maelstrom {
  struct Request;
  struct Response;

  struct Error;

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

  struct Message {
    std::string source;
    std::string destination;
    nlohmann::json body;

    [[nodiscard]] bool isRequest() const;
    std::optional<Request> toRequest() &&;

    [[nodiscard]] bool isResponse() const;
    std::optional<Response> toResponse() &&;

    nlohmann::json toJson() &&;

    static std::optional<Message> parse(std::string raw_message);
  };

  struct Request {
    std::string source;
    std::string destination;
    std::string type;
    nlohmann::json body;
    std::uint64_t message_id;

    Message toMessage() &&;

    Response toResponse(nlohmann::json body) &&;
    Response toResponse() &&;

    Error toError(ErrorCode code, std::string what = {}) &&;
  };

  struct Response {
    std::string source;
    std::string destination;
    std::string type;
    nlohmann::json body;
    std::uint64_t in_reply_to;

    Message toMessage() &&;

    [[nodiscard]] bool isError() const;
    std::optional<Error> toError() &&;
  };

  struct Error {
    std::string source;
    std::string destination;
    std::string type;
    ErrorCode code;
    std::string what;
    nlohmann::json body;
    std::uint64_t in_reply_to;

    Response toResponse() &&;
  };
}// namespace maelstrom