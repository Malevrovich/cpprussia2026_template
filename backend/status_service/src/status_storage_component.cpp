#include "status_storage_component.hpp"
#include <userver/components/component.hpp>
#include <userver/utils/datetime.hpp>

namespace status_service {

StatusStorageComponent::StatusStorageComponent(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : userver::components::ComponentBase(config, context) {
  // Component initialization
}

void StatusStorageComponent::StoreStatus(
    const V1Login& login, const V1UserStatus& status,
    std::optional<std::chrono::system_clock::time_point> expires_at) {
  std::lock_guard lock(mutex_);

  // Clean up expired records before storing new one
  CleanupExpired();

  UserStatusRecord record;
  record.status = status;
  record.updated_at = std::chrono::system_clock::now();
  record.expires_at = expires_at;

  storage_[login] = record;
}

std::optional<UserStatusRecord> StatusStorageComponent::GetStatus(
    const V1Login& login) {
  std::lock_guard lock(mutex_);

  // Clean up expired records before retrieving
  CleanupExpired();

  auto it = storage_.find(login);
  if (it == storage_.end()) {
    return std::nullopt;
  }

  // Check if the record is expired
  if (it->second.IsExpired()) {
    storage_.erase(it);
    return std::nullopt;
  }

  return it->second;
}

bool StatusStorageComponent::UserExists(const V1Login& login) {
  std::lock_guard lock(mutex_);

  // Clean up expired records before checking
  CleanupExpired();

  auto it = storage_.find(login);
  if (it == storage_.end()) {
    return false;
  }

  // Check if the record is expired
  if (it->second.IsExpired()) {
    storage_.erase(it);
    return false;
  }

  return true;
}

void StatusStorageComponent::CleanupExpired() {
  auto now = std::chrono::system_clock::now();

  for (auto it = storage_.begin(); it != storage_.end();) {
    if (it->second.IsExpired()) {
      it = storage_.erase(it);
    } else {
      ++it;
    }
  }
}

}  // namespace status_service