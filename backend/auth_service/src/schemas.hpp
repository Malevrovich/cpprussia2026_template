#pragma once

#include <optional>
#include <string>
#include <unordered_map>

namespace auth_service {

// V1Login - Unique user ID in the system
using V1Login = std::string;

// V1CurrentUser - Current user information (without token)
struct V1CurrentUser {
  V1Login login;
  std::string name;
};

// V1AuthorizedUser - Authorized user with JWT token
struct V1AuthorizedUser {
  V1Login login;
  std::string name;
  std::string token;  // JWT token, 128 characters Base64-encoded
};

// V1Error - Structure to report any error
struct V1Error {
  std::string code;
  std::string message;
  std::optional<std::unordered_map<std::string, std::string>> details;
};

// V1UserRegistrationRequest - Registration request
struct V1UserRegistrationRequest {
  V1Login login;
  std::string name;
  std::string email;
  std::string phone;
  std::string password;  // min 6 characters
};

// V1UserAuthorizationRequest - Authorization request
struct V1UserAuthorizationRequest {
  V1Login login;
  std::string password;
};

// V1UserAuthorizationResponse - Response after successful
// authorization/registration
struct V1UserAuthorizationResponse {
  V1AuthorizedUser current_user;
};

// V1UserRegistrationResponse is same as V1UserAuthorizationResponse
using V1UserRegistrationResponse = V1UserAuthorizationResponse;

}  // namespace auth_service