#include "handlers/broadcast.hpp"

#include <yaclib/async/make.hpp>

namespace ds::broadcast {
  yaclib::Future<core::Response>
  BroadcastHandler::handle(core::Network::Session&& session,
                           core::Request&& request) {
    auto& storage = env_->state->storage;

    auto new_num = request.body["message"].get<std::uint64_t>();

    {
      std::lock_guard guard{storage.mtx};
      if (storage.unique_nums.find(new_num) == storage.unique_nums.end()) {
        storage.unique_nums.insert(new_num);
        storage.nums.push_back(new_num);
      }
    }

    return yaclib::MakeFuture(std::move(request).toResponse());
  }

  yaclib::Future<core::Response>
  BroadcastBulkHandler::handle(core::Network::Session&& session,
                               core::Request&& request) {
    auto& storage = env_->state->storage;

    auto new_nums = request.body["messages"].get<std::vector<std::uint64_t>>();

    {
      std::lock_guard guard{storage.mtx};

      for (auto new_num: new_nums) {
        if (storage.unique_nums.find(new_num) == storage.unique_nums.end()) {
          storage.unique_nums.insert(new_num);
          storage.nums.push_back(new_num);
        }
      }
    }

    return yaclib::MakeFuture(std::move(request).toResponse());
  }
}// namespace ds::broadcast