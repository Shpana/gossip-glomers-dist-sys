#include "handlers/read.hpp"

#include <yaclib/async/make.hpp>

namespace ds::broadcast {
  yaclib::Future<core::Response>
  ReadHandler::handle([[maybe_unused]] core::Network::Session&& session,
                      core::Request&& request) {
    auto& storage = state_->storage;

    auto body = nlohmann::json({});

    {
      std::lock_guard guard{storage.mtx};
      body["messages"] = storage.unique_nums;
    }

    return yaclib::MakeFuture(std::move(request).toResponse(std::move(body)));
  }
}// namespace ds::broadcast