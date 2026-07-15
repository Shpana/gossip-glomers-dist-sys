#include "ops.hpp"

#include <yaclib/async/make.hpp>
#include <yaclib/coro/await.hpp>
#include <yaclib/coro/future.hpp>

#include "network/detail/errors.hpp"
#include "network/messages.hpp"
#include "network/services/key_value_storage.hpp"

namespace ds::gcounter {
  yaclib::Future<maelstrom::Response>
  AddHandler::handle(maelstrom::Network::Session&& session,
                     maelstrom::Request&& reqeust) {
    auto delta = reqeust.body["delta"].get<std::int64_t>();

    auto kv_storage = maelstrom::KeyValueStorage<
        std::string, std::int64_t,
        maelstrom::Consistency::SequentialConsistent>(session);

    co_await kv_storage.read("atomic");

    co_return yaclib::MakeFuture(std::move(reqeust).toResponse());

    // kv_storage.read("atomic").ThenInline(
    //     [kv_storage = std::move(kv_storage)](
    //         std::expected<std::int64_t, maelstrom::Error> r) mutable {
    //       if (!r.has_value()) {
    //         kv_storage.write("atomic", 0)
    //             .ThenInline([kv_storage = std::move(kv_storage)](
    //                             std::optional<maelstrom::Error> r) mutable {

    //             });
    //       }
    //     });
  }

  yaclib::Future<maelstrom::Response>
  ReadHandler::handle(maelstrom::Network::Session&& session,
                      maelstrom::Request&& reqeust) {}
}// namespace ds::gcounter