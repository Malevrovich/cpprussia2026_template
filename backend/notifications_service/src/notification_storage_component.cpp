#include "notification_storage_component.hpp"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <userver/components/component.hpp>

namespace notifications_service {

NotificationStorageComponent::NotificationStorageComponent(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : userver::components::ComponentBase(config, context) {
  // Component initialization
}

std::string NotificationStorageComponent::CreateNotification(
    V1ChannelId channel_id, V1MessageId message_id,
    const V1Login& target_user_login, const V1Login& sender_login) {
  std::string notification_id = GenerateUuid();

  NotificationRecord record{.notification_id = notification_id,
                            .channel_id = channel_id,
                            .message_id = message_id,
                            .target_user_login = target_user_login,
                            .sender_login = sender_login,
                            .read = false,
                            .created_at = std::chrono::system_clock::now()};

  {
    std::lock_guard lock(mutex_);

    // Add to channel->user map
    auto& user_map = notifications_by_channel_and_user_[channel_id];
    auto& user_notifications = user_map[target_user_login];
    user_notifications.push_back(record);

    // Store pointer for quick lookup
    notification_by_id_[notification_id] = &user_notifications.back();
  }

  return notification_id;
}

std::vector<V1NotificationStatus>
NotificationStorageComponent::GetUserNotifications(V1ChannelId channel_id,
                                                   const V1Login& user_login) {
  std::vector<V1NotificationStatus> result;

  std::lock_guard lock(mutex_);

  auto channel_it = notifications_by_channel_and_user_.find(channel_id);
  if (channel_it == notifications_by_channel_and_user_.end()) {
    return result;  // No notifications for this channel
  }

  auto user_it = channel_it->second.find(user_login);
  if (user_it == channel_it->second.end()) {
    return result;  // No notifications for this user in this channel
  }

  const auto& notifications = user_it->second;
  result.reserve(notifications.size());

  for (const auto& record : notifications) {
    result.push_back(V1NotificationStatus{.message_id = record.message_id,
                                          .read = record.read});
  }

  return result;
}

bool NotificationStorageComponent::MarkAsRead(
    const std::string& notification_id) {
  std::lock_guard lock(mutex_);

  auto it = notification_by_id_.find(notification_id);
  if (it == notification_by_id_.end()) {
    return false;
  }

  it->second->read = true;
  return true;
}

bool NotificationStorageComponent::MarkNotificationAsRead(
    V1ChannelId channel_id, V1MessageId message_id, const V1Login& user_login) {
  std::lock_guard lock(mutex_);

  auto channel_it = notifications_by_channel_and_user_.find(channel_id);
  if (channel_it == notifications_by_channel_and_user_.end()) {
    return false;
  }

  auto user_it = channel_it->second.find(user_login);
  if (user_it == channel_it->second.end()) {
    return false;
  }

  bool found = false;
  for (auto& record : user_it->second) {
    if (record.message_id == message_id && !record.read) {
      record.read = true;
      found = true;
      // Don't break, mark all matching notifications as read
    }
  }

  return found;
}

bool NotificationStorageComponent::NotificationExists(
    const std::string& notification_id) const {
  std::lock_guard lock(mutex_);
  return notification_by_id_.find(notification_id) != notification_by_id_.end();
}

std::string NotificationStorageComponent::GenerateUuid() const {
  static boost::uuids::random_generator generator;
  boost::uuids::uuid uuid = generator();
  return boost::uuids::to_string(uuid);
}

}  // namespace notifications_service