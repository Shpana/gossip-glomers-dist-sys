#pragma once

#include <expected>

#include "network/network.hpp"

namespace ds::core {
  enum struct Consistency : uint8_t {
    Linearizable = 0,
    SequentialConsistent,
    LastWriteWins
  };

  namespace detail {
    template<Consistency C>
    struct Type;
    template<>
    struct Type<Consistency::Linearizable> {
      static constexpr std::string_view type = "lin-kv";
    };
    template<>
    struct Type<Consistency::SequentialConsistent> {
      static constexpr std::string_view type = "seq-kv";
    };
    template<>
    struct Type<Consistency::LastWriteWins> {
      static constexpr std::string_view type = "lww-kv";
    };
  }// namespace detail

  template<typename K, typename V, Consistency C>
  class KeyValueStorage : public detail::Type<C> {
  public:
    struct ReadHandler {
      static constexpr std::string_view type = "read";
    };
    struct WriteHandler {
      static constexpr std::string_view type = "write";
    };
    struct CompareAndSwapHandler {
      static constexpr std::string_view type = "cas";
    };

  public:
    explicit KeyValueStorage(Network::Session& session);

    yaclib::Future<std::expected<V, Error>> read(K key);
    yaclib::Future<std::optional<Error>> write(K key, V value);
    yaclib::Future<std::optional<Error>>
    compareAndSwap(K key, V from, V to, bool create_if_not_exists = true);

  private:
    Network::Session& session_;
  };

  template<typename K, typename V, Consistency C>
  KeyValueStorage<K, V, C>::KeyValueStorage(Network::Session& session)
      : session_{session} {}

  template<typename K, typename V, Consistency C>
  yaclib::Future<std::expected<V, Error>>
  KeyValueStorage<K, V, C>::read(K key) {
    auto body = nlohmann::json({});
    body["key"] = std::hash<K>(std::move(key));

    return session_
        .call<KeyValueStorage::ReadHandler>(KeyValueStorage::type,
                                            std::move(body))
        .ThenInline([](yaclib::Result<Response> result) {
          if (!result) {
            return std::move(result).Error();
          }

          auto response = std::move(result).Ok();

          if (response.isError()) {
            auto error = std::move(response).toError().value();
            return std::unexpected{error};
          }

          return std::move(response);
        });
  }

  template<typename K, typename V, Consistency C>
  yaclib::Future<std::optional<Error>>
  KeyValueStorage<K, V, C>::write(K key, V value) {
    auto body = nlohmann::json({});
    body["key"] = std::hash<K>(std::move(key));
    body["value"] = std::move(value);

    return session_
        .call<KeyValueStorage::WriteHandler>(KeyValueStorage::type,
                                             std::move(body))
        .ThenInline([](yaclib::Result<Response> result) {
          if (!result) {
            return std::move(result).Error();
          }

          auto response = std::move(result).Ok();

          if (response.isError()) {
            auto error = std::move(response).toError().value();
            return std::unexpected{error};
          }

          return std::nullopt;
        });
  }

  template<typename K, typename V, Consistency C>
  yaclib::Future<std::optional<Error>>
  KeyValueStorage<K, V, C>::compareAndSwap(K key, V from, V to,
                                           bool create_if_not_exists) {
    auto body = nlohmann::json({});
    body["key"] = std::hash<K>(std::move(key));
    body["from"] = std::move(from);
    body["to"] = std::move(to);
    body["create_if_not_exists"] = create_if_not_exists;

    return session_
        .call<KeyValueStorage::CompareAndSwapHandler>(KeyValueStorage::type,
                                                      std::move(body))
        .ThenInline([](yaclib::Result<Response> result) {
          if (!result) {
            return std::move(result).Error();
          }

          auto response = std::move(result).Ok();

          if (response.isError()) {
            auto error = std::move(response).toError().value();
            return std::unexpected{error};
          }

          return std::nullopt;
        });
  }
}// namespace ds::core