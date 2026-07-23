#include "operations.hpp"

#include "log/logging.hpp"
#include "network/messages.hpp"
#include "network/services/kv_storage.hpp"

#include <yaclib/async/when_all.hpp>
#include <yaclib/coro/await.hpp>
#include <yaclib/coro/future.hpp>

using namespace std::chrono_literals;

namespace tasks::gcounter {
namespace {
constexpr State::Clock::duration kCacheInvalidationTimeout{1s};
} // namespace

static yaclib::Future<maelstrom::Response>
AddHandler::Handle(maelstrom::Network::Session &&session,
                   maelstrom::Request &&request) {
  auto &local = state_->local;

  auto delta = request.body["delta"].get<std::uint64_t>();

  auto kv_storage = maelstrom::KeyValueStorage<
    std::uint64_t, maelstrom::Consistency::SequentialConsistent>{session};

  // Try to find the best estimation to minimize messages count
  std::uint64_t guess = local.value.fetch_add(delta);

  while ((co_await kv_storage.compareAndSwap(env_->node_id, guess,
                                             guess + delta, true))
           .has_value()) {
    guess = (co_await kv_storage.read(env_->node_id)).value();
  }

  co_return std::move(request).toResponse();
}

static yaclib::Future<maelstrom::Response>
ReadHandler::Handle(maelstrom::Network::Session &&session,
                    maelstrom::Request &&request) {
  auto &cache = state_->cache;

  if (cache.enabled.load()) {
    auto guard = co_await cache.mtx.Guard();

    if (cache.valid_to.has_value() &&
        cache.valid_to.value() > State::Clock::now()) {
      // Caching for eventual consistency is ok
      auto body = nlohmann::json({});
      body["value"] = cache.value;
      co_return std::move(request).toResponse(std::move(body));
    }
  }

  auto value = co_await read(session);

  if (cache.enabled.load()) {
    auto guard = co_await cache.mtx.Guard();
    cache.value = value;
    cache.valid_to = State::Clock::now() + cache_invalidation_timeout;
  }

  auto body = nlohmann::json({});
  body["value"] = value;
  co_return std::move(request).toResponse(std::move(body));
}

static yaclib::Future<std::uint64_t>
ReadHandler::Read(maelstrom::Network::Session &session) {
  auto kv_storage = maelstrom::KeyValueStorage<
    std::uint64_t, maelstrom::Consistency::SequentialConsistent>{session};

  {
    using Result = std::optional<maelstrom::Error>;
    std::vector<yaclib::Future<Result>> fs;
    fs.reserve(env_->available_node_ids.size());

    for (auto &id : env_->available_node_ids) {
      // To garentee 'program order' for reads
      // Sequantial consistent systems may not provide last value for reading
      // without such calls
      fs.push_back(kv_storage.compareAndSwap(id, 0, 0));
    }

    co_await yaclib::WhenAll(fs.begin(), fs.end());
  }

  std::uint64_t value;

  {
    using Result = std::expected<std::uint64_t, maelstrom::Error>;
    std::vector<yaclib::Future<Result>> fs;
    fs.reserve(env_->available_node_ids.size());

    for (auto &id : env_->available_node_ids) {
      fs.push_back(kv_storage.read(id));
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
} // namespace tasks::gcounter