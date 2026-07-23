#include "handlers.hpp"

#include <yaclib/coro/future.hpp>

namespace tasks::kafka::part1 {
  namespace {
    constexpr std::size_t max_poll_size{10};
  }// namespace

  yaclib::Future<maelstrom::Response>
  SendHandler::handle(maelstrom::Network::Session session,
                      maelstrom::Request request) {
    auto state = getState();

    auto key = request.body["key"].get<std::string>();
    auto message = request.body["msg"].get<Message>();

    auto offset = state->next_offset.fetch_add(1);

    {
      auto guard = co_await state->mtx.Guard();
      state->logs[key].insert({offset, message});
    }

    auto body = nlohmann::json({});
    body["offset"] = offset;
    co_return std::move(request).toResponse(std::move(body));
  }

  yaclib::Future<maelstrom::Response>
  PollHandler::handle(maelstrom::Network::Session session,
                      maelstrom::Request request) {
    auto state = getState();

    auto offsets = request.body["offsets"].get<std::map<std::string, Offset>>();

    auto body = nlohmann::json({});
    body["msgs"] = nlohmann::json({});

    {
      auto guard = co_await state->mtx.GuardShared();

      for (const auto& [key, offset_bound]: offsets) {
        if (!state->logs.contains(key)) {
          continue;
        }

        const auto& log = state->logs[key];

        if (log.lower_bound(offset_bound) == log.end()) {
          continue;
        }

        body["msgs"][key] = nlohmann::json::array({});
        for (const auto& [offset, message]:
             std::ranges::subrange{log.lower_bound(offset_bound), log.end()} |
                 std::views::take(max_poll_size)) {
          body["msgs"][key].push_back(nlohmann::json::array({offset, message}));
        }
      }
    }

    co_return std::move(request).toResponse(std::move(body));
  }

  yaclib::Future<maelstrom::Response>
  CommitOffsetsHandler::handle(maelstrom::Network::Session session,
                               maelstrom::Request request) {
    auto state = getState();

    auto offsets = request.body["offsets"].get<std::map<std::string, Offset>>();

    {
      auto guard = co_await state->mtx.Guard();

      for (const auto& [key, offset]: offsets) {
        state->committed[key] = offset;
      }
    }

    co_return std::move(request).toResponse();
  }

  yaclib::Future<maelstrom::Response>
  ListCommittedOffsetsHandler::handle(maelstrom::Network::Session session,
                                      maelstrom::Request request) {
    auto state = getState();

    auto keys = request.body["keys"].get<std::vector<std::string>>();

    auto body = nlohmann::json({});
    body["offsets"] = nlohmann::json({});

    {
      auto guard = co_await state->mtx.GuardShared();

      for (const auto& key: keys) {
        if (!state->committed.contains(key)) {
          continue;
        }

        body["offsets"][key] = state->committed[key];
      }
    }

    co_return std::move(request).toResponse(std::move(body));
  }
}// namespace tasks::kafka::part1