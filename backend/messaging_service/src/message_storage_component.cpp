#include "message_storage_component.hpp"
#include <algorithm>
#include <userver/components/component.hpp>
#include <userver/utils/datetime.hpp>

namespace messaging_service {

MessageStorageComponent::MessageStorageComponent(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : userver::components::ComponentBase(config, context) {
  // Component initialization if needed
}

V1MessageId MessageStorageComponent::StoreMessage(
    V1ChannelId channel_id, const V1ChannelMessage& message) {
  std::lock_guard lock(mutex_);

  // Get or create the channel's message vector
  auto& channel_messages = messages_by_channel_[channel_id];

  // Store the message
  channel_messages.push_back(message);

  return message.id;
}

std::vector<V1ChannelMessage> MessageStorageComponent::GetMessagesByTimestamp(
    V1ChannelId channel_id, const std::string& from,
    const std::optional<std::string>& to, size_t limit) {
  std::lock_guard lock(mutex_);

  auto it = messages_by_channel_.find(channel_id);
  if (it == messages_by_channel_.end()) {
    // Channel exists but has no messages yet
    return {};
  }

  const auto& messages = it->second;
  std::vector<V1ChannelMessage> result;

  // Simple filtering by timestamp
  // In a real implementation, we would parse timestamps and compare properly
  // For now, we'll return all messages up to the limit
  size_t count = 0;
  for (const auto& msg : messages) {
    if (count >= limit) break;

    // Basic timestamp comparison (lexicographical for ISO8601 strings)
    if (msg.timestamp >= from) {
      if (!to.has_value() || msg.timestamp <= to.value()) {
        result.push_back(msg);
        ++count;
      }
    }
  }

  return result;
}

bool MessageStorageComponent::ChannelExists(V1ChannelId /*channel_id*/) const {
  // According to spec, all channel IDs already exist
  return true;
}

V1MessageId MessageStorageComponent::GetNextMessageId() {
  return next_message_id_.fetch_add(1, std::memory_order_relaxed);
}

}  // namespace messaging_service