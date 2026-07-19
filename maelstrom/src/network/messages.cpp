#include "network/messages.hpp"

#include "log/logging.hpp"

namespace maelstrom {
  namespace {
    constexpr std::string_view error_type = "error";
  }

  bool Message::isRequest() const {
    return body.contains("type") && body.contains("msg_id");
  }

  std::optional<Request> Message::toRequest() && {
    if (!isRequest()) {
      return std::nullopt;
    }

    auto type = body["type"].get<std::string>();
    auto message_id = body["msg_id"].get<std::uint64_t>();

    return Request{.source = std::move(source),
                   .destination = std::move(destination),
                   .type = std::move(type),
                   .body = std::move(body),
                   .message_id = message_id};
  }

  bool Message::isResponse() const {
    return body.contains("type") && body.contains("in_reply_to");
  }

  std::optional<Response> Message::toResponse() && {
    if (!isResponse()) {
      return std::nullopt;
    }

    auto type = body["type"].get<std::string>();
    auto in_reply_to = body["in_reply_to"].get<std::uint64_t>();

    return Response{.source = std::move(source),
                    .destination = std::move(destination),
                    .type = std::move(type),
                    .body = std::move(body),
                    .in_reply_to = in_reply_to};
  }

  nlohmann::json Message::toJson() && {
    nlohmann::json json_message;
    json_message["src"] = std::move(source);
    json_message["dest"] = std::move(destination);
    json_message["body"] = std::move(body);
    return json_message;
  }

  std::optional<Message> Message::parse(std::string raw_message) {
    auto json = nlohmann::json::parse(std::move(raw_message));

    Message request;

    if (!json.contains("src") || !json.contains("dest") ||
        !json.contains("body")) {
      return std::nullopt;
    }

    return Message{.source = std::move(json["src"].get<std::string>()),
                   .destination = std::move(json["dest"].get<std::string>()),
                   .body = std::move(json["body"])};
  }

  Message Request::toMessage() && {
    body["type"] = std::move(type);
    body["msg_id"] = message_id;

    return Message{.source = std::move(source),
                   .destination = std::move(destination),
                   .body = std::move(body)};
  }

  Response Request::toResponse(nlohmann::json body) && {
    return Response{.source = std::move(destination),
                    .destination = std::move(source),
                    .type = type + "_ok",
                    .body = std::move(body),
                    .in_reply_to = message_id};
  }

  Response Request::toResponse() && {
    return std::move(*this).toResponse(nlohmann::json({}));
  }

  Error Request::toError(ErrorCode code, std::string what) && {
    return Error{.source = std::move(destination),
                 .destination = std::move(source),
                 .code = code,
                 .what = std::move(what),
                 .body = nlohmann::json({}),
                 .in_reply_to = message_id};
  }

  Message Response::toMessage() && {
    body["type"] = std::move(type);
    body["in_reply_to"] = in_reply_to;

    return Message{.source = std::move(source),
                   .destination = std::move(destination),
                   .body = std::move(body)};
  }

  bool Response::isError() const {
    return type == error_type && body.contains("code") && body.contains("text");
  }

  std::optional<Error> Response::toError() && {
    if (!isError()) {
      return std::nullopt;
    }

    return Error{.source = std::move(source),
                 .destination = std::move(destination),
                 .code = body["code"].get<ErrorCode>(),
                 .what = std::move(body["text"].get<std::string>()),
                 .body = std::move(body),
                 .in_reply_to = in_reply_to};
  }

  Response Error::toResponse() && {
    body["code"] = code;
    body["text"] = std::move(what);

    return Response{.source = std::move(source),
                    .destination = std::move(destination),
                    .type = std::string{error_type},
                    .body = std::move(body),
                    .in_reply_to = in_reply_to};
  }
}// namespace maelstrom