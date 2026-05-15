#pragma once

#include <optional>
#include <string>
#include <vector>

namespace reactions_service {

// V1ChannelId - Channel ID in the system
using V1ChannelId = int64_t;

// V1MessageId - Unique message ID for a specific channel
using V1MessageId = int64_t;

// V1Login - Unique user ID in the system
using V1Login = std::string;

// V1Animation - Animation type for reaction
enum class V1Animation { kLike, kDislike, kHeart, kFire, kOkay, kLol, kSmile };

// Convert V1Animation to string
std::string ToString(V1Animation animation);

// Convert string to V1Animation (throws on invalid)
V1Animation AnimationFromString(const std::string& str);

// V1CurrentUser - Current user information
struct V1CurrentUser {
  std::string token;  // 128 characters
  std::string login;  // min 3 characters
  std::string name;   // human readable name
};

// V1LikeTriggerRequest - Request to toggle a reaction
struct V1LikeTriggerRequest {
  V1CurrentUser current_user;
  std::string idempotency_token;  // 16-256 characters
  V1ChannelId channel_id;
  V1MessageId message_id;
  V1Animation animation;
};

// V1LikeTriggerResponse - Response after toggle
struct V1LikeTriggerResponse {
  enum class Action { kAdded, kRemoved };

  Action action;
  std::optional<V1Animation> current_user_reaction;  // null if no reaction
};

// V1ReactionEntry - A single reaction entry
struct V1ReactionEntry {
  V1Login user;
  V1Animation animation;
};

// V1GetReactionsResponse - Response with all reactions on a message
struct V1GetReactionsResponse {
  std::vector<V1ReactionEntry> reactions;
};

// V1Error - Error response
struct V1Error {
  int32_t code;
  std::string message;
};

}  // namespace reactions_service