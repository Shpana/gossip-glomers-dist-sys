#pragma once

#include <memory>
#include <string_view>
#include <vector>

#include "messages.hpp"
#include "transport.hpp"

namespace ds::core {
  struct Context {
    const std::string& node_id;
    const std::vector<std::string>& available_node_ids;
  };

  class IHandler {
  public:
    virtual ~IHandler() = default;

    [[nodiscard]] virtual std::string_view name() const = 0;
    virtual Response handle(Context&& context, Request&& request) = 0;
  };

  class Node {
    struct Environment {
      std::string node_id;
      std::vector<std::string> available_node_ids;
    };

  public:
    Node();

    void run();
    void registerHandler(std::unique_ptr<IHandler> handler);

  private:
    void handle(Request&& request);
    void handleInit(Request&& request);

  private:
    std::unique_ptr<Transport> transport_;
    std::optional<Environment> environment_{std::nullopt};

    std::unordered_map<std::string, std::unique_ptr<IHandler>> handlers_{};
  };
}// namespace ds::core