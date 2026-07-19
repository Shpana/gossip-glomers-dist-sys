#pragma once

#include "network/messages.hpp"
#include "network/network.hpp"
#include "routines/handler.hpp"

#include "state.hpp"

namespace tasks::kafka::part2 {
  class SendHandler final : public maelstrom::HandlerBase<State> {
  public:
    static constexpr std::string_view type{"send"};

    yaclib::Future<maelstrom::Response>
    handle(maelstrom::Network::Session session,
           maelstrom::Request request) override;

  private:
    yaclib::Future<Offset> insertToLog(maelstrom::Network::Session& session,
                                       const std::string& key, Message message);
  };

  class PollHandler final : public maelstrom::HandlerBase<State> {
  public:
    static constexpr std::string_view type{"poll"};

    yaclib::Future<maelstrom::Response>
    handle(maelstrom::Network::Session session,
           maelstrom::Request request) override;
  };

  class CommitOffsetsHandler final : public maelstrom::HandlerBase<State> {
  public:
    static constexpr std::string_view type{"commit_offsets"};

    yaclib::Future<maelstrom::Response>
    handle(maelstrom::Network::Session session,
           maelstrom::Request request) override;
  };

  class ListCommittedOffsetsHandler final
      : public maelstrom::HandlerBase<State> {
  public:
    static constexpr std::string_view type{"list_committed_offsets"};

    yaclib::Future<maelstrom::Response>
    handle(maelstrom::Network::Session session,
           maelstrom::Request request) override;
  };
}// namespace tasks::kafka::part2