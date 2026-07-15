#pragma once

#include "routines/handler.hpp"
#include "utils/unit.hpp"

namespace ds::gcounter {
  class AddHandler final : public core::HandlerBase<core::Unit> {
  public:
    static constexpr std::string_view type = "add";

    yaclib::Future<core::Response> handle(core::Network::Session&& session,
                                          core::Request&& reqeust) override;
  };

  class ReadHandler final : public core::HandlerBase<core::Unit> {
  public:
    static constexpr std::string_view type = "read";

    yaclib::Future<core::Response> handle(core::Network::Session&& session,
                                          core::Request&& reqeust) override;
  };
}// namespace ds::gcounter