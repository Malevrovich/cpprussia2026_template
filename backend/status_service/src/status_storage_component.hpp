#pragma once

#include <chrono>
#include <optional>
#include <unordered_map>
#include <userver/components/component_base.hpp>
#include <userver/engine/mutex.hpp>
#include "schemas.hpp"

namespace status_service {

// Internal record stored in memory
struct UserStatusRecord {
  V1UserStatus status;
  std::chrono::system_clock::time_point updated_at;
  std::optional<std::chrono::system_clock::time_point> expires_at;

  bool IsExpired() const {
    if (!expires_at.has_value()) {
      return false;
    }
    return std::chrono::system_clock::now() > *expires_at;
  }
};

class StatusStorageComponent final : public userver::components::ComponentBase {
 public:
  static constexpr std::string_view kName = "status-storage";

  StatusStorageComponent(const userver::components::ComponentConfig& config,
                         const userver::components::ComponentContext& context);

  // Store or update user status
  void StoreStatus(const V1Login& login, const V1UserStatus& status,
                   std::optional<std::chrono::system_clock::time_point>
                       expires_at = std::nullopt);

  // Get user status if exists and not expired
  std::optional<UserStatusRecord> GetStatus(const V1Login& login);

  // Check if user exists (has any status, even expired)
  bool UserExists(const V1Login& login);

 private:
  // Remove expired records from storage
  void CleanupExpired();

  // Thread-safe storage
  mutable userver::engine::Mutex mutex_;
  std::unordered_map<V1Login, UserStatusRecord> storage_;
};

}  // namespace status_service