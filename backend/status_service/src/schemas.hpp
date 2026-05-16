#pragma once

#include <chrono>
#include <optional>
#include <string>

namespace status_service {

// V1Login - Unique user ID in the system
using V1Login = std::string;

// V1StatusType - Type of user status
enum class V1StatusType { kOnline, kAway, kBusy, kOffline };

// V1Visibility - Visibility of the status
enum class V1Visibility { kPublic, kPrivate };

// V1CurrentUser - Current user information, including authorization token if
// authorized
struct V1CurrentUser {
  std::string token;  // 128 characters, empty if not authorized
  V1Login login;      // min 3 characters
  std::string name;   // human readable name
};

// V1UserStatus - User status with defined schema
struct V1UserStatus {
  V1StatusType status_type;
  std::string status_message;
  V1Visibility visibility{V1Visibility::kPublic};
};

// V1UserStatusUpdateRequest - Request to update user status
struct V1UserStatusUpdateRequest {
  V1CurrentUser current_user;
  V1UserStatus status;
};

// V1UserStatusUpdateResponse - Response after updating status
struct V1UserStatusUpdateResponse {
  bool success;
  std::chrono::system_clock::time_point updated_at;
  std::optional<std::chrono::system_clock::time_point> expires_at;
};

// V1UserStatusByLoginRequest - Request to get user status by login
struct V1UserStatusByLoginRequest {
  V1CurrentUser current_user;
  V1Login login;
};

// V1UserStatusByLoginResponse - Response with user status
struct V1UserStatusByLoginResponse {
  V1UserStatus status;
  std::chrono::system_clock::time_point updated_at;
};

// V1ErrorResponse - Error response structure
struct V1ErrorResponse {
  std::string error;
  std::string message;
};

}  // namespace status_service