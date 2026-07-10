#pragma once

#include <memory>
#include <string_view>
#include <vector>

#include "messages.hpp"
#include "transport.hpp"

namespace ds::core {
  struct Environment {
    std::string node_id;
    std::vector<std::string> available_node_ids;
  };

  class IHandler {
  public:
    virtual ~IHandler() = default;

    [[nodiscard]] virtual std::string_view type() const = 0;

    virtual Response handle(Request&& request) = 0;

    virtual void start(Environment env) {}
    virtual void stop() {}
  };

  class Node {
  public:
    Node();

    void run();

    template<typename Handler>
    void add() {
      auto handler = std::make_unique<Handler>();
      handlers_[std::string{handler->type()}] = std::move(handler);
    }

  private:
    void handle(Request&& request);
    void handleInit(Request&& request);

  private:
    std::unique_ptr<Transport> transport_;
    std::optional<Environment> env_{std::nullopt};

    std::unordered_map<std::string, std::unique_ptr<IHandler>> handlers_{};

    bool was_started_{false};
  };
}// namespace ds::core