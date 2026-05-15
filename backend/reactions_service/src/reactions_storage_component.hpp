#pragma once

#include <atomic>
#include <map>
#include <optional>
#include <string>
#include <tuple>
#include <userver/components/component_base.hpp>
#include <userver/engine/mutex.hpp>
#include <vector>
#include "schemas.hpp"

namespace reactions_service {

class ReactionsStorageComponent final
    : public userver::components::ComponentBase {
 public:
  static constexpr std::string_view kName = "reactions-storage";

  ReactionsStorageComponent(
      const userver::components::ComponentConfig& config,
      const userver::components::ComponentContext& context);

  /// Toggle a reaction on a message with idempotency token support.
  /// Returns the action performed (added/removed) and the user's current
  /// reaction.
  std::pair<V1LikeTriggerResponse::Action, std::optional<V1Animation>>
  ToggleReaction(const V1LikeTriggerRequest& request);

  /// Get all reactions on a specific message.
  std::vector<V1ReactionEntry> GetReactions(V1ChannelId channel_id,
                                            V1MessageId message_id) const;

  /// Check if a message exists (according to spec, all messages exist).
  bool MessageExists(V1ChannelId channel_id, V1MessageId message_id) const;

 private:
  struct IdempotencyRecord {
    V1Login user;
    V1ChannelId channel_id;
    V1MessageId message_id;
    V1Animation animation;
    V1LikeTriggerResponse::Action action;
  };

  mutable userver::engine::Mutex mutex_;

  // channel_id -> message_id -> user_login -> animation
  std::map<V1ChannelId, std::map<V1MessageId, std::map<V1Login, V1Animation>>>
      reactions_;

  // idempotency_token -> record
  std::map<std::string, IdempotencyRecord> idempotency_tokens_;
};

}  // namespace reactions_service