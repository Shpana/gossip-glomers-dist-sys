#pragma once

#include "state.hpp"

#include <maelstrom/network/messages.hpp>
#include <maelstrom/network/network.hpp>
#include <maelstrom/routines/handler.hpp>
#include <maelstrom/utils/unit.hpp>

namespace tasks::kafka::part2 {

class SendHandler final : public maelstrom::HandlerBase<maelstrom::Unit> {
public:
  static constexpr std::string_view kType{"send"};

  yaclib::Future<maelstrom::Response>
  Handle(maelstrom::Network::Session session,
         maelstrom::Request request) override;

private:
  yaclib::Future<Offset> InsertToLog(maelstrom::Network::Session &session,
                                     const std::string &key, Message message);
};

class PollHandler final : public maelstrom::HandlerBase<maelstrom::Unit> {
public:
  static constexpr std::string_view kType{"poll"};

  yaclib::Future<maelstrom::Response>
  Handle(maelstrom::Network::Session session,
         maelstrom::Request request) override;
};

class CommitOffsetsHandler final
  : public maelstrom::HandlerBase<maelstrom::Unit> {
public:
  static constexpr std::string_view kType{"commit_offsets"};

  yaclib::Future<maelstrom::Response>
  Handle(maelstrom::Network::Session session,
         maelstrom::Request request) override;
};

class ListCommittedOffsetsHandler final
  : public maelstrom::HandlerBase<maelstrom::Unit> {
public:
  static constexpr std::string_view kType{"list_committed_offsets"};

  yaclib::Future<maelstrom::Response>
  Handle(maelstrom::Network::Session session,
         maelstrom::Request request) override;
};

} // namespace tasks::kafka::part2