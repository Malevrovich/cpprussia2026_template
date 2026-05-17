#pragma once

#include <optional>
#include <string>
#include <vector>

namespace notifications_service {

// V1ChannelId - Channel ID in the system
using V1ChannelId = int64_t;

// V1MessageId - Unique message ID for a specific channel
using V1MessageId = int64_t;

// V1Login - Unique user login (username) in the system
using V1Login = std::string;

// V1NotificationId - Unique notification ID (UUID)
using V1NotificationId = std::string;

// V1CurrentUser - Current user information
struct V1CurrentUser {
  std::string token;  // 128 characters, empty if not authorized
  std::string login;  // min 3 characters
  std::string name;   // human readable name
};

// V1NotificationStatus - Notification status with message ID and read flag
struct V1NotificationStatus {
  V1MessageId message_id;
  bool read;
};

// V1ChannelNotificationNewRequest - Request to create a new notification
struct V1ChannelNotificationNewRequest {
  V1CurrentUser current_user;
  V1ChannelId channel_id;
  V1MessageId message_id;
  V1Login other_user_login;
};

// V1ChannelNotificationNewResponse - Response after creating a notification
struct V1ChannelNotificationNewResponse {
  V1NotificationId notification_id;
};

// V1ChannelNotificationListRequest - Request to list notifications
struct V1ChannelNotificationListRequest {
  V1CurrentUser current_user;
  V1ChannelId channel_id;
};

// V1ChannelNotificationListResponse - Response with notifications list
struct V1ChannelNotificationListResponse {
  std::vector<V1NotificationStatus> notifications;
};

// V1ChannelNotificationReadRequest - Request to mark a notification as read
struct V1ChannelNotificationReadRequest {
  V1CurrentUser current_user;
  V1ChannelId channel_id;
  V1MessageId message_id;
};

// V1ChannelNotificationReadResponse - Response after marking a notification as
// read
struct V1ChannelNotificationReadResponse {
  bool ok;
};

// V1Error - Standard error response
struct V1Error {
  std::string error;
  std::string message;
};

}  // namespace notifications_service