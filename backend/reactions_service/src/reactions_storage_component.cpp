#include "reactions_storage_component.hpp"

#include <userver/components/component.hpp>
#include <userver/logging/log.hpp>

namespace reactions_service {

ReactionsStorageComponent::ReactionsStorageComponent(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : userver::components::ComponentBase(config, context) {
  LOG_INFO() << "ReactionsStorageComponent initialized";
}

std::pair<V1LikeTriggerResponse::Action, std::optional<V1Animation>>
ReactionsStorageComponent::ToggleReaction(const V1LikeTriggerRequest& request) {
  std::lock_guard lock(mutex_);

  // Check idempotency token first
  auto it_token = idempotency_tokens_.find(request.idempotency_token);
  if (it_token != idempotency_tokens_.end()) {
    const auto& record = it_token->second;
    // Check if token matches the same parameters
    if (record.user == request.current_user.login &&
        record.channel_id == request.channel_id &&
        record.message_id == request.message_id &&
        record.animation == request.animation) {
      // Token matches - return current state without changes
      auto& user_reactions = reactions_[request.channel_id][request.message_id];
      auto it_user = user_reactions.find(request.current_user.login);
      if (it_user != user_reactions.end()) {
        return {record.action, it_user->second};
      } else {
        return {record.action, std::nullopt};
      }
    } else {
      // Token exists but with different parameters - conflict
      throw std::runtime_error("Idempotency token conflict");
    }
  }

  // No token or token mismatch - perform toggle
  auto& user_reactions = reactions_[request.channel_id][request.message_id];
  auto it_user = user_reactions.find(request.current_user.login);

  V1LikeTriggerResponse::Action action;
  std::optional<V1Animation> current_reaction;

  if (it_user != user_reactions.end()) {
    // User already has a reaction on this message
    if (it_user->second == request.animation) {
      // Same animation - remove it
      user_reactions.erase(it_user);
      action = V1LikeTriggerResponse::Action::kRemoved;
      current_reaction = std::nullopt;
    } else {
      // Different animation - replace it
      it_user->second = request.animation;
      action = V1LikeTriggerResponse::Action::kAdded;
      current_reaction = request.animation;
    }
  } else {
    // No existing reaction - add new one
    user_reactions[request.current_user.login] = request.animation;
    action = V1LikeTriggerResponse::Action::kAdded;
    current_reaction = request.animation;
  }

  // Store idempotency token record
  idempotency_tokens_[request.idempotency_token] =
      IdempotencyRecord{.user = request.current_user.login,
                        .channel_id = request.channel_id,
                        .message_id = request.message_id,
                        .animation = request.animation,
                        .action = action};

  return {action, current_reaction};
}

std::vector<V1ReactionEntry> ReactionsStorageComponent::GetReactions(
    V1ChannelId channel_id, V1MessageId message_id) const {
  std::lock_guard lock(mutex_);

  std::vector<V1ReactionEntry> result;

  auto it_channel = reactions_.find(channel_id);
  if (it_channel == reactions_.end()) {
    return result;  // No reactions in this channel
  }

  auto it_message = it_channel->second.find(message_id);
  if (it_message == it_channel->second.end()) {
    return result;  // No reactions on this message
  }

  for (const auto& [user, animation] : it_message->second) {
    result.push_back(V1ReactionEntry{.user = user, .animation = animation});
  }

  return result;
}

bool ReactionsStorageComponent::MessageExists(
    [[maybe_unused]] V1ChannelId channel_id,
    [[maybe_unused]] V1MessageId message_id) const {
  // According to spec, all channel IDs and message IDs already exist
  // We don't need to validate existence
  return true;
}

}  // namespace reactions_service