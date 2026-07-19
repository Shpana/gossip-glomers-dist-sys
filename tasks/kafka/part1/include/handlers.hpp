#pragma once

#include "routines/handler.hpp"

#include "state.hpp"

namespace tasks::kafka::part1 {
  class SendHandler final : public maelstrom::HandlerBase<State> {
  public:
    static constexpr std::string_view type{"send"};

    yaclib::Future<maelstrom::Response>
    handle(maelstrom::Network::Session&& session,
           maelstrom::Request&& request) override;
  };

  class PollHandler final : public maelstrom::HandlerBase<State> {
  public:
    static constexpr std::string_view type{"poll"};

    yaclib::Future<maelstrom::Response>
    handle(maelstrom::Network::Session&& session,
           maelstrom::Request&& request) override;
  };

  class CommitOffsetsHandler final : public maelstrom::HandlerBase<State> {
  public:
    static constexpr std::string_view type{"commit_offsets"};

    yaclib::Future<maelstrom::Response>
    handle(maelstrom::Network::Session&& session,
           maelstrom::Request&& request) override;
  };

  class ListCommittedOffsetsHandler final
      : public maelstrom::HandlerBase<State> {
  public:
    static constexpr std::string_view type{"list_committed_offsets"};

    yaclib::Future<maelstrom::Response>
    handle(maelstrom::Network::Session&& session,
           maelstrom::Request&& request) override;
  };
}// namespace tasks::kafka::part1