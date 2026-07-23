#pragma once

#include <expected>

#include <yaclib/async/make.hpp>

#include <maelstrom/network/network.hpp>

namespace maelstrom {

enum struct Consistency : uint8_t {
  Linearizable = 0,
  SequentialConsistent,
  LastWriteWins
};

namespace detail {

template <Consistency C> struct StorageType;

template <> struct StorageType<Consistency::Linearizable> {
  static constexpr std::string_view type = "lin-kv";
};

template <> struct StorageType<Consistency::SequentialConsistent> {
  static constexpr std::string_view type = "seq-kv";
};

template <> struct StorageType<Consistency::LastWriteWins> {
  static constexpr std::string_view type = "lww-kv";
};

} // namespace detail

template <typename V, Consistency C>
class KeyValueStorage : public detail::StorageType<C> {
public:
  class ReadHandler;
  class WriteHandler;
  class CompareAndSwapHandler;

public:
  explicit KeyValueStorage(Network::Session &session);

  yaclib::Future<std::expected<V, Error>>
  read(std::string key,
       std::optional<Network::Clock::duration> timeout = std::nullopt);
  yaclib::Future<std::optional<Error>>
  write(std::string key, V value,
        std::optional<Network::Clock::duration> timeout = std::nullopt);
  yaclib::Future<std::optional<Error>> compareAndSwap(
      std::string key, V from, V to, bool create_if_not_exists = true,
      std::optional<Network::Clock::duration> timeout = std::nullopt);

private:
  Network::Session &session_;
};

template <typename V, Consistency C> class KeyValueStorage<V, C>::ReadHandler {
  static constexpr std::string_view type = "read";
};

template <typename V, Consistency C> class KeyValueStorage<V, C>::WriteHandler {
  static constexpr std::string_view type = "write";
};

template <typename V, Consistency C>
class KeyValueStorage<V, C>::CompareAndSwapHandler {
  static constexpr std::string_view type = "cas";
};

} // namespace maelstrom

template <typename V, maelstrom::Consistency C>
maelstrom::KeyValueStorage<V, C>::KeyValueStorage(Network::Session &session)
    : session_{session} {}

template <typename V, maelstrom::Consistency C>
yaclib::Future<std::expected<V, maelstrom::Error>>
maelstrom::KeyValueStorage<V, C>::read(
    std::string key, std::optional<Network::Clock::duration> timeout) {
  auto body = nlohmann::json({});
  body["key"] = std::move(key);

  auto response = co_await session_.call<ReadHandler>(
      std::string{KeyValueStorage::type}, std::move(body), timeout);

  if (response.isError()) {
    co_return std::unexpected{std::move(response).toError().value()};
  }

  co_return response.body["value"].template get<V>();
}

template <typename V, maelstrom::Consistency C>
yaclib::Future<std::optional<maelstrom::Error>>
maelstrom::KeyValueStorage<V, C>::write(
    std::string key, V value, std::optional<Network::Clock::duration> timeout) {
  auto body = nlohmann::json({});
  body["key"] = std::move(key);
  body["value"] = std::move(value);

  auto response = co_await session_.call<WriteHandler>(
      std::string{KeyValueStorage::type}, std::move(body), timeout);

  if (response.isError()) {
    co_return std::move(response).toError().value();
  }

  co_return std::nullopt;
}

template <typename V, maelstrom::Consistency C>
yaclib::Future<std::optional<maelstrom::Error>>
maelstrom::KeyValueStorage<V, C>::compareAndSwap(
    std::string key, V from, V to, bool create_if_not_exists,
    std::optional<Network::Clock::duration> timeout) {
  auto body = nlohmann::json({});
  body["key"] = std::move(key);
  body["from"] = std::move(from);
  body["to"] = std::move(to);
  body["create_if_not_exists"] = create_if_not_exists;

  auto response = co_await session_.call<CompareAndSwapHandler>(
      std::string{KeyValueStorage::type}, std::move(body), timeout);

  if (response.isError()) {
    co_return std::move(response).toError().value();
  }

  co_return std::nullopt;
}