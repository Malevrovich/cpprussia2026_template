#pragma once

#include <atomic>
#include <map>
#include <optional>
#include <string>
#include <userver/components/component_base.hpp>
#include <userver/engine/mutex.hpp>
#include <vector>
#include "schemas.hpp"

namespace messaging_service {

class MessageStorageComponent final
    : public userver::components::ComponentBase {
 public:
  static constexpr std::string_view kName = "message-storage";

  MessageStorageComponent(const userver::components::ComponentConfig& config,
                          const userver::components::ComponentContext& context);

  /// Store a new message in the specified channel.
  /// Returns the assigned message ID.
  V1MessageId StoreMessage(V1ChannelId channel_id,
                           const V1ChannelMessage& message);

  /// Retrieve messages from a channel within a timestamp range.
  /// If 'to' is not provided, uses current time.
  /// Results are sorted by timestamp ascending.
  std::vector<V1ChannelMessage> GetMessagesByTimestamp(
      V1ChannelId channel_id, const std::string& from,
      const std::optional<std::string>& to, size_t limit);

  /// Check if a channel exists.
  /// According to spec, all channel IDs already exist.
  bool ChannelExists(V1ChannelId channel_id) const;

  /// Get the next message ID (thread-safe)
  V1MessageId GetNextMessageId();

 private:
  mutable userver::engine::Mutex mutex_;
  std::map<V1ChannelId, std::vector<V1ChannelMessage>> messages_by_channel_;
  std::atomic<V1MessageId> next_message_id_{1};
};

}  // namespace messaging_service