#pragma once

#include <atomic>
#include <chrono>
#include <map>
#include <optional>
#include <string>
#include <userver/components/component_base.hpp>
#include <userver/engine/mutex.hpp>
#include <vector>
#include "schemas.hpp"

namespace notifications_service {

// Internal notification record with additional metadata
struct NotificationRecord {
  std::string notification_id;  // UUID
  V1ChannelId channel_id;
  V1MessageId message_id;
  V1Login target_user_login;  // User being notified
  V1Login sender_login;       // User who created notification
  bool read;
  std::chrono::system_clock::time_point created_at;
};

class NotificationStorageComponent final
    : public userver::components::ComponentBase {
 public:
  static constexpr std::string_view kName = "notification-storage";

  NotificationStorageComponent(
      const userver::components::ComponentConfig& config,
      const userver::components::ComponentContext& context);

  /// Create a new notification for a user in a channel.
  /// Returns the generated notification ID (UUID).
  std::string CreateNotification(V1ChannelId channel_id, V1MessageId message_id,
                                 const V1Login& target_user_login,
                                 const V1Login& sender_login);

  /// Get all notifications for a user in a specific channel.
  /// Returns vector of notification statuses (message_id + read flag).
  std::vector<V1NotificationStatus> GetUserNotifications(
      V1ChannelId channel_id, const V1Login& user_login);

  /// Mark a notification as read.
  /// Returns true if notification was found and updated.
  bool MarkAsRead(const std::string& notification_id);

  /// Check if a notification exists.
  bool NotificationExists(const std::string& notification_id) const;

 private:
  // Generate a UUID string
  std::string GenerateUuid() const;

  mutable userver::engine::Mutex mutex_;

  // Storage structure: channel_id -> target_user_login ->
  // vector<NotificationRecord>
  std::map<V1ChannelId, std::map<V1Login, std::vector<NotificationRecord>>>
      notifications_by_channel_and_user_;

  // Quick lookup by notification ID for updates
  std::map<std::string, NotificationRecord*> notification_by_id_;
};

}  // namespace notifications_service