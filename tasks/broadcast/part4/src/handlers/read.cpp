#include "handlers/read.hpp"

#include <yaclib/async/make.hpp>

namespace ds::broadcast {
  yaclib::Future<maelstrom::Response>
  ReadHandler::handle([[maybe_unused]] maelstrom::Network::Session&& session,
                      maelstrom::Request&& request) {
    auto& storage = env_->state->storage;

    auto body = nlohmann::json({});

    {
      std::lock_guard guard{storage.mtx};
      body["messages"] = storage.unique_nums;
    }

    return yaclib::MakeFuture(std::move(request).toResponse(std::move(body)));
  }
}// namespace ds::broadcast