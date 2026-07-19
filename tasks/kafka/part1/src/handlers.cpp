#include "handlers.hpp"

#include <yaclib/coro/future.hpp>

namespace tasks::kafka::part1 {
  namespace {
    constexpr std::size_t max_poll_size{10};
  }// namespace

  yaclib::Future<maelstrom::Response>
  SendHandler::handle(maelstrom::Network::Session&& session,
                      maelstrom::Request&& request) {
    auto key = request.body["key"].get<std::string>();
    auto message = request.body["msg"].get<Message>();

    auto offset = state_->next_offset.fetch_add(1);

    {
      auto guard = co_await state_->mtx.Guard();
      state_->logs[key].insert({offset, message});
    }

    auto body = nlohmann::json({});
    body["offset"] = offset;
    co_return std::move(request).toResponse(std::move(body));
  }

  yaclib::Future<maelstrom::Response>
  PollHandler::handle(maelstrom::Network::Session&& session,
                      maelstrom::Request&& request) {
    auto offsets = request.body["offsets"].get<std::map<std::string, Offset>>();

    auto body = nlohmann::json({});
    body["msgs"] = nlohmann::json({});

    {
      auto guard = co_await state_->mtx.GuardShared();

      for (auto&& [key, offset_bound]: offsets) {
        if (!state_->logs.contains(key)) {
          continue;
        }

        const auto& log = state_->logs[key];

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
  CommitOffsetsHandler::handle(maelstrom::Network::Session&& session,
                               maelstrom::Request&& request) {
    auto offsets = request.body["offsets"].get<std::map<std::string, Offset>>();

    {
      auto guard = co_await state_->mtx.Guard();

      for (auto&& [key, offset]: offsets) {
        state_->committed[key] = std::max(offset, state_->committed[key]);
      }
    }

    co_return std::move(request).toResponse();
  }


  yaclib::Future<maelstrom::Response>
  ListCommittedOffsetsHandler::handle(maelstrom::Network::Session&& session,
                                      maelstrom::Request&& request) {
    auto keys = request.body["keys"].get<std::vector<std::string>>();

    auto body = nlohmann::json({});
    body["offsets"] = nlohmann::json({});

    {
      auto guard = co_await state_->mtx.GuardShared();

      for (auto&& key: keys) {
        if (!state_->committed.contains(key)) {
          continue;
        }

        body["offsets"][key] = state_->committed[key];
      }
    }

    co_return std::move(request).toResponse(std::move(body));
  }
}// namespace tasks::kafka::part1