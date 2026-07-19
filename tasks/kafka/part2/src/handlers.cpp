#include "handlers.hpp"

#include <yaclib/async/future.hpp>
#include <yaclib/async/when_all.hpp>
#include <yaclib/coro/await.hpp>
#include <yaclib/coro/future.hpp>

#include "log/logging.hpp"
#include "network/messages.hpp"
#include "network/services/kv_storage.hpp"

namespace tasks::kafka::part2 {
  namespace {
    constexpr std::size_t max_poll_size{10};
  }// namespace

  yaclib::Future<maelstrom::Response>
  SendHandler::handle(maelstrom::Network::Session session,
                      maelstrom::Request request) {
    auto key = request.body["key"].get<std::string>();
    auto message = request.body["msg"].get<Message>();

    auto offset = co_await insertToLog(session, key, message);

    auto body = nlohmann::json({});
    body["offset"] = offset;
    co_return std::move(request).toResponse(std::move(body));
  }

  yaclib::Future<Offset>
  SendHandler::insertToLog(maelstrom::Network::Session& session,
                           const std::string& key, Message message) {
    auto log_storage = maelstrom::KeyValueStorage<
        Log, maelstrom::Consistency::SequentialConsistent>{session};

    Offset offset{0};

    Log guess{};

    Log updated{};
    updated.insert({offset, message});

    while ((co_await log_storage.compareAndSwap("log_" + key, guess, updated,
                                                true))
               .has_value()) {
      guess = std::move(co_await log_storage.read("log_" + key)).value();

      offset = (*guess.rbegin()).first + 1;

      updated = guess;
      updated.insert({offset, message});
    }

    co_return offset;
  }

  yaclib::Future<maelstrom::Response>
  PollHandler::handle(maelstrom::Network::Session session,
                      maelstrom::Request request) {
    auto offsets = request.body["offsets"].get<std::map<std::string, Offset>>();

    auto body = nlohmann::json({});
    body["msgs"] = nlohmann::json({});

    auto log_storage = maelstrom::KeyValueStorage<
        Log, maelstrom::Consistency::SequentialConsistent>{session};

    using Result = std::expected<Log, maelstrom::Error>;
    std::vector<yaclib::Future<Result>> fs;
    fs.reserve(offsets.size());

    for (const auto& [key, _]: offsets) {
      fs.push_back(log_storage.read("log_" + key));
    }

    if (fs.empty()) {
      co_return std::move(request).toResponse(std::move(body));
    }

    auto results = co_await yaclib::WhenAll(fs.begin(), fs.end());

    for (const auto& [result, elem]: std::views::zip(results, offsets)) {
      if (!result.has_value()) {
        continue;
      }

      const auto& log = result.value();

      const auto& key = elem.first;
      auto offset_bound = elem.second;

      if (log.lower_bound(offset_bound) != log.end()) {
        continue;
      }

      body["msgs"][key] = nlohmann::json::array({});
      for (const auto& [offset, message]:
           std::ranges::subrange{log.lower_bound(offset_bound), log.end()} |
               std::views::take(max_poll_size)) {
        body["msgs"][key].push_back(nlohmann::json::array({offset, message}));
      }
    }

    co_return std::move(request).toResponse(std::move(body));
  }

  yaclib::Future<maelstrom::Response>
  CommitOffsetsHandler::handle(maelstrom::Network::Session session,
                               maelstrom::Request request) {
    auto offsets = request.body["offsets"].get<Committed>();

    auto committed_storage = maelstrom::KeyValueStorage<
        Committed, maelstrom::Consistency::Linearizable>{session};

    Committed guess{};
    Committed update = offsets;

    while ((co_await committed_storage.compareAndSwap(
                "committed", std::move(guess), std::move(update), true))
               .has_value()) {
      guess = (co_await committed_storage.read("committed")).value();
      update = guess;
      for (const auto& [key, offset]: offsets) {
        update[key] = std::max(offset, update[key]);
      }
    }

    co_return std::move(request).toResponse();
  }

  yaclib::Future<maelstrom::Response>
  ListCommittedOffsetsHandler::handle(maelstrom::Network::Session session,
                                      maelstrom::Request request) {
    auto keys = request.body["keys"].get<std::vector<std::string>>();

    auto body = nlohmann::json({});
    body["offsets"] = nlohmann::json({});

    auto committed_storage = maelstrom::KeyValueStorage<
        Committed, maelstrom::Consistency::Linearizable>{session};

    auto result = co_await committed_storage.read("committed");

    if (!result.has_value()) {
      co_return std::move(request).toResponse(std::move(body));
    }

    const auto& committed = result.value();

    for (auto&& key: keys) {
      if (!committed.contains(key)) {
        continue;
      }

      body["offsets"][key] = committed.at(key);
    }

    co_return std::move(request).toResponse(std::move(body));
  }
}// namespace tasks::kafka::part2