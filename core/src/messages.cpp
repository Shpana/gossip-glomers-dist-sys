#include "messages.hpp"

namespace ds::core {
  std::optional<Request> Request::parse(std::string raw_request) {
    auto json = nlohmann::json::parse(std::move(raw_request));

    Request request;

    if (!json.contains("src") || !json.contains("dest") ||
        !json.contains("body")) {
      return std::nullopt;
    }

    request.sender = json["src"].get<std::string>();
    request.recipient = json["dest"].get<std::string>();
    request.body = json.at("body");

    const auto& body = request.body;

    if (!body.contains("type") || !body.contains("msg_id")) {
      return std::nullopt;
    }

    request.type = body["type"].get<std::string>();
    request.message_id = body["msg_id"].get<std::uint64_t>();

    return request;
  }

  Response Request::reply(std::string type, nlohmann::json body) const {
    body["type"] = type;
    body["in_reply_to"] = message_id;

    return Response{.sender = recipient,
                    .recipient = sender,
                    .type = std::move(type),
                    .body = std::move(body),
                    .in_reply_to = message_id};
  }

  Response Request::reply(std::string type) const {
    return reply(std::move(type), nlohmann::json({}));
  }

  Response Request::reply(std::string_view type, nlohmann::json body) const {
    return reply(std::string{type}, std::move(body));
  }

  Response Request::reply(std::string_view type) const {
    return reply(type, nlohmann::json({}));
  }
}// namespace ds::core