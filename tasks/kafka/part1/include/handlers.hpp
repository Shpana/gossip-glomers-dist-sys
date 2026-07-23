#pragma once

#include "state.hpp"

#include <maelstrom/routines/handler.hpp>

namespace tasks::kafka::part1 {

class SendHandler final : public maelstrom::HandlerBase<State> {
public:
  static constexpr std::string_view kType{"send"};

  yaclib::Future<maelstrom::Response>
  Handle(maelstrom::Network::Session session,
         maelstrom::Request request) override;
};

class PollHandler final : public maelstrom::HandlerBase<State> {
public:
  static constexpr std::string_view kType{"poll"};

  yaclib::Future<maelstrom::Response>
  Handle(maelstrom::Network::Session session,
         maelstrom::Request request) override;
};

class CommitOffsetsHandler final : public maelstrom::HandlerBase<State> {
public:
  static constexpr std::string_view kType{"commit_offsets"};

  yaclib::Future<maelstrom::Response>
  Handle(maelstrom::Network::Session session,
         maelstrom::Request request) override;
};

class ListCommittedOffsetsHandler final : public maelstrom::HandlerBase<State> {
public:
  static constexpr std::string_view kType{"list_committed_offsets"};

  yaclib::Future<maelstrom::Response>
  Handle(maelstrom::Network::Session session,
         maelstrom::Request request) override;
};

} // namespace tasks::kafka::part1