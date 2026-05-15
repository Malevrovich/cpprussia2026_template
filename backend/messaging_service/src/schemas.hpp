#pragma once

#include <optional>
#include <string>
#include <vector>

namespace messaging_service {

// V1ChannelId - Channel ID in the system
using V1ChannelId = int64_t;

// V1MessageId - Unique message ID for a specific channel
using V1MessageId = int64_t;

// V1CurrentUser - Current user information
struct V1CurrentUser {
  std::string token;  // 128 characters
  std::string login;  // min 3 characters
  std::string name;   // human readable name
};

// V1ChannelMessage - Structure of a message in a channel
struct V1ChannelMessage {
  V1CurrentUser current_user;
  V1MessageId id;
  std::string timestamp;  // ISO8601 format
  std::string message;    // min 1 character
};

// V1ChannelMessageNewRequest - Request to create a new message
struct V1ChannelMessageNewRequest {
  V1CurrentUser current_user;
  V1ChannelId channel_id;
  std::string message;
};

// V1ChannelMessageNewResponse - Response after creating a new message
struct V1ChannelMessageNewResponse {
  V1MessageId message_id;
};

// V1ChannelMessageByTimestampRequest - Request to get messages by timestamp
struct V1ChannelMessageByTimestampRequest {
  V1ChannelId channel_id;
  std::string from;               // required, ISO8601
  std::optional<std::string> to;  // optional, ISO8601
  int32_t limit = 100;            // 1-1000, default 100
};

// V1ChannelMessageByTimestampResponse - Response with messages
struct V1ChannelMessageByTimestampResponse {
  std::vector<V1ChannelMessage> messages;
  std::optional<std::string> next_cursor;  // null if no more results
  bool has_more = false;
};

// V1Error - Error structure
struct V1Error {
  std::string error;
  int32_t code;
};

}  // namespace messaging_service