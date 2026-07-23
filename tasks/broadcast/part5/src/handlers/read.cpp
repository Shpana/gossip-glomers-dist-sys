#include "handlers/read.hpp"

#include <yaclib/async/make.hpp>

namespace tasks::broadcast::part5 {

yaclib::Future<maelstrom::Response>
ReadHandler::Handle(maelstrom::Network::Session session,
                    maelstrom::Request request) {
  auto &storage = GetState().storage;

  auto body = nlohmann::json({});

  {
    std::lock_guard guard{storage.mtx};
    body["messages"] = storage.unique_nums;
  }

  return yaclib::MakeFuture(std::move(request).ToResponse(std::move(body)));
}

} // namespace tasks::broadcast::part5