#include "ops.hpp"

#include <yaclib/async/make.hpp>
#include <yaclib/coro/await.hpp>
#include <yaclib/coro/future.hpp>

#include "network/detail/errors.hpp"
#include "network/messages.hpp"
#include "network/services/key_value_storage.hpp"

namespace ds::gcounter {
  yaclib::Future<core::Response>
  AddHandler::handle(core::Network::Session&& session,
                     core::Request&& reqeust) {
    auto delta = reqeust.body["delta"].get<std::int64_t>();

    auto kv_storage =
        core::KeyValueStorage<std::string, std::int64_t,
                              core::Consistency::SequentialConsistent>(session);

    co_await kv_storage.read("atomic");

    co_return yaclib::MakeFuture(std::move(reqeust).toResponse());

    // kv_storage.read("atomic").ThenInline(
    //     [kv_storage = std::move(kv_storage)](
    //         std::expected<std::int64_t, core::Error> r) mutable {
    //       if (!r.has_value()) {
    //         kv_storage.write("atomic", 0)
    //             .ThenInline([kv_storage = std::move(kv_storage)](
    //                             std::optional<core::Error> r) mutable {

    //             });
    //       }
    //     });
  }

  yaclib::Future<core::Response>
  ReadHandler::handle(core::Network::Session&& session,
                      core::Request&& reqeust) {}
}// namespace ds::gcounter