#include "node.hpp"

#include <future>
#include <iostream>
#include <memory>

#include <fmt/format.h>

namespace ds::core {
  constexpr std::string_view kInitType = "init";
  constexpr std::string_view kInitOkType = "init_ok";

  Node::Node() : transport_{std::make_unique<Transport>()} {}

  void Node::run() {
    while (transport_->isRunning()) {
      auto request = transport_->recieve();
      if (!request.has_value()) {
        continue;
      }
      handle(std::move(request.value()));
    }

    for (auto& [type, handler]: handlers_) {
      handler->stop();
    }
  }

  void Node::handle(Request&& request) {
    const auto& type = request.type;

    if (type == kInitType) {
      return handleInit(std::move(request));
    }

    if (auto it = handlers_.find(type); it != handlers_.end()) {
      if (!was_started_) {
        std::cerr << "There is request to not initializated node!\n";
        return;
      }

      // TODO(shpana): add better task execution
      auto f = std::async(std::launch::async,
                          [this, &it, request = std::move(request)]() mutable {
                            auto& handler = it->second;
                            auto response = handler->handle(std::move(request));

                            transport_->send(std::move(response));
                          });
      return;
    }

    std::cerr << fmt::format("No handler for message type '{}'\n", type);
  }

  void Node::handleInit(Request&& request) {
    auto& body = request.body;

    auto node_id = body["node_id"].get<std::string>();
    auto available_node_ids = body["node_ids"].get<std::vector<std::string>>();

    env_.emplace(
        Environment{.node_id = std::move(node_id),
                    .available_node_ids = std::move(available_node_ids)});

    for (auto& [type, handler]: handlers_) {
      handler->start(env_.value());
    }

    std::exchange(was_started_, true);

    transport_->send(request.reply(kInitOkType));
  }
}// namespace ds::core