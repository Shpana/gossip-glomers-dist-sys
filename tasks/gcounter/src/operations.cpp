#include "operations.hpp"

#include <yaclib/async/when_all.hpp>
#include <yaclib/coro/await.hpp>
#include <yaclib/coro/future.hpp>

#include <maelstrom/log/logging.hpp>
#include <maelstrom/network/messages.hpp>
#include <maelstrom/network/services/kv_storage.hpp>

namespace tasks::gcounter {

namespace {
using namespace std::chrono_literals;
constexpr State::Clock::duration kCacheInvalidationTimeout{1s};
} // namespace

yaclib::Future<maelstrom::Response>
AddHandler::Handle(maelstrom::Network::Session session,
                   maelstrom::Request request) {
  auto &local = GetState().local;

  auto delta = request.body["delta"].get<std::uint64_t>();

  auto kv_storage = maelstrom::KeyValueStorage<
    std::uint64_t, maelstrom::Consistency::SequentialConsistent>{session};

  // Try to find the best estimation to minimize messages count
  std::uint64_t guess = local.value.fetch_add(delta);

  while ((co_await kv_storage.CompareAndSwap(GetEnvironment().node_id, guess,
                                             guess + delta, true))
           .has_value()) {
    guess = (co_await kv_storage.Read(GetEnvironment().node_id)).value();
  }

  co_return std::move(request).ToResponse();
}

yaclib::Future<maelstrom::Response>
ReadHandler::Handle(maelstrom::Network::Session session,
                    maelstrom::Request request) {
  auto &cache = GetState().cache;

  if (cache.enabled.load()) {
    auto guard = co_await cache.mtx.Guard();

    if (cache.valid_to.has_value() &&
        cache.valid_to.value() > State::Clock::now()) {
      // Caching for eventual consistency is ok
      auto body = nlohmann::json({});
      body["value"] = cache.value;
      co_return std::move(request).ToResponse(std::move(body));
    }
  }

  auto value = co_await Read(session);

  if (cache.enabled.load()) {
    auto guard = co_await cache.mtx.Guard();
    cache.value = value;
    cache.valid_to = State::Clock::now() + kCacheInvalidationTimeout;
  }

  auto body = nlohmann::json({});
  body["value"] = value;
  co_return std::move(request).ToResponse(std::move(body));
}

yaclib::Future<std::uint64_t>
ReadHandler::Read(maelstrom::Network::Session &session) {
  auto kv_storage = maelstrom::KeyValueStorage<
    std::uint64_t, maelstrom::Consistency::SequentialConsistent>{session};

  std::uint64_t value;

  {
    using Result = std::expected<std::uint64_t, maelstrom::Error>;
    std::vector<yaclib::Future<Result>> fs;
    fs.reserve(GetEnvironment().available_node_ids.size());

    for (auto &id : GetEnvironment().available_node_ids) {
      fs.push_back(kv_storage.Read(id));
    }

    auto results = co_await yaclib::WhenAll(fs.begin(), fs.end());

    value =
      std::accumulate(results.begin(), results.end(), 0ull,
                      [](std::uint64_t acc, const Result &result) {
                        return acc + (result.has_value() ? result.value() : 0);
                      });
  }

  co_return value;
}

yaclib::Future<> OrdererWorker::Process(maelstrom::Network::Session session) {
  auto kv_storage = maelstrom::KeyValueStorage<
    std::uint64_t, maelstrom::Consistency::SequentialConsistent>{session};

  {
    using Result = std::optional<maelstrom::Error>;
    std::vector<yaclib::Future<Result>> fs;
    fs.reserve(GetEnvironment().available_node_ids.size());

    for (auto &id : GetEnvironment().available_node_ids) {
      // To garentee 'program order' for reads
      // Sequantial consistent systems may not provide last value for reading
      // without such calls
      fs.push_back(kv_storage.CompareAndSwap(id, 0, 0));
    }

    co_await yaclib::WhenAll(fs.begin(), fs.end());
  }

  co_return {};
}

} // namespace tasks::gcounter