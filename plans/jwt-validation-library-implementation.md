# JWT Validation Library Implementation Plan

## Overview
Create a shared JWT token validation library in `backend/common/` for use across all microservices. The library will provide "dumb" validation (checking token length and non-emptiness) as a foundation for future real JWT validation.

## Current State Analysis

### Services with Token Validation
1. **status_service**: Has `ValidateToken()` method in `status_update_handler.cpp`
   - Checks: token not empty, length >= 128 characters
   - Throws: `userver::server::handlers::Unauthorized` with JSON error response

2. **Other services** (messaging, notifications, files, reactions, auth):
   - No token validation implemented
   - Only basic field validation (e.g., login not empty)

### Common Structures
All services use similar `V1CurrentUser` struct:
```cpp
struct V1CurrentUser {
  std::string token;  // 128 characters
  std::string login;  // min 3 characters  
  std::string name;   // human readable name
};
```

## Library Design

### Directory Structure
```
backend/common/
├── jwt_validation/
│   ├── jwt_validator.hpp
│   ├── jwt_validator.cpp
│   └── CMakeLists.txt
└── CMakeLists.txt (root)
```

### Interface Design (`jwt_validator.hpp`)
```cpp
#pragma once

#include <string>
#include <userver/server/handlers/exceptions.hpp>

namespace common::jwt {

class JwtValidator {
 public:
  // Validates token, throws Unauthorized if invalid
  static void ValidateToken(const std::string& token);
  
  // Returns true if token is valid
  static bool IsValidToken(const std::string& token);
  
  // Extracts login from token (dummy implementation for now)
  static std::string ExtractLoginFromToken(const std::string& token);
  
  // Validates token and extracts login
  static std::string ValidateAndExtractLogin(const std::string& token);

 private:
  static constexpr size_t kMinTokenLength = 128;
};

} // namespace common::jwt
```

### Implementation (`jwt_validator.cpp`)
```cpp
#include "jwt_validator.hpp"
#include <userver/formats/json.hpp>
#include <userver/formats/serialize/common_containers.hpp>

namespace common::jwt {

namespace {

// Error response structure matching existing services
struct ErrorResponse {
  std::string error;
  std::string message;
};

// Serialization for ErrorResponse
template <typename Value>
Value Serialize(const ErrorResponse& response,
                userver::formats::serialize::To<Value>) {
  typename Value::Builder builder;
  builder["error"] = response.error;
  builder["message"] = response.message;
  return builder.ExtractValue();
}

} // namespace

void JwtValidator::ValidateToken(const std::string& token) {
  if (token.empty()) {
    throw userver::server::handlers::Unauthorized(
        userver::server::handlers::ExternalBody{
            userver::formats::json::ToString(
                Serialize(ErrorResponse{"unauthorized", "Token is required"},
                          userver::formats::serialize::To<
                              userver::formats::json::Value>{}))});
  }

  if (token.length() < kMinTokenLength) {
    throw userver::server::handlers::Unauthorized(
        userver::server::handlers::ExternalBody{
            userver::formats::json::ToString(
                Serialize(ErrorResponse{"unauthorized", "Invalid token format"},
                          userver::formats::serialize::To<
                              userver::formats::json::Value>{}))});
  }
}

bool JwtValidator::IsValidToken(const std::string& token) {
  return !token.empty() && token.length() >= kMinTokenLength;
}

std::string JwtValidator::ExtractLoginFromToken(const std::string& token) {
  // Dummy implementation - in real JWT, we would parse the payload
  // For now, return empty string or implement simple extraction if needed
  return "";
}

std::string JwtValidator::ValidateAndExtractLogin(const std::string& token) {
  ValidateToken(token);
  return ExtractLoginFromToken(token);
}

} // namespace common::jwt
```

## Build System Updates

### Root `backend/common/CMakeLists.txt`
```cmake
cmake_minimum_required(VERSION 3.16)
project(common)

add_subdirectory(jwt_validation)
```

### `backend/common/jwt_validation/CMakeLists.txt`
```cmake
add_library(jwt_validation STATIC
  jwt_validator.cpp
)

target_include_directories(jwt_validation PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(jwt_validation PUBLIC
  userver::core
  userver::server
  userver::formats_json
)
```

### Service CMakeLists.txt Updates
Each service's CMakeLists.txt needs to:
1. Add `backend/common` as a subdirectory
2. Link against `jwt_validation` library

Example for status_service:
```cmake
# Add common library
add_subdirectory(${CMAKE_SOURCE_DIR}/../common)

# Link the library
target_link_libraries(${PROJECT_NAME} PRIVATE
  jwt_validation
  # ... existing dependencies
)
```

## Integration Plan

### 1. Auth Service
- Add token validation to authorization endpoints
- Use `JwtValidator::ValidateToken()` for incoming tokens in future endpoints

### 2. Status Service
- Replace existing `ValidateToken()` method with library version
- Update `status_update_handler.cpp` and `status_by_login_handler.cpp`

### 3. Messaging Service
- Add token validation to `message_new_handler.cpp`
- Add token validation to `message_by_timestamp_handler.cpp`

### 4. Notifications Service
- Add token validation to `notification_new_handler.cpp`
- Add token validation to `notification_list_handler.cpp`

### 5. Files Service
- Add token validation to `file_new_handler.cpp`
- Add token validation to `file_by_uri_handler.cpp`

### 6. Reactions Service
- Add token validation to `get_reactions_handler.cpp`
- Add token validation to `like_trigger_handler.cpp`

## Integration Example

### Before (status_service):
```cpp
void StatusUpdateHandler::ValidateToken(const std::string& token) const {
  if (token.empty()) {
    throw userver::server::handlers::Unauthorized(...);
  }
  if (token.length() < 128) {
    throw userver::server::handlers::Unauthorized(...);
  }
}
```

### After:
```cpp
#include <common/jwt_validation/jwt_validator.hpp>

// In HandleRequestThrow method:
common::jwt::JwtValidator::ValidateToken(request.current_user.token);
```

## Testing Strategy

1. **Unit Tests**: Test `JwtValidator` class directly
2. **Integration Tests**: Update existing service tests to include token validation
3. **Negative Tests**: Test with empty tokens, short tokens, invalid tokens

## Future Enhancements

1. **Real JWT Validation**: Add proper JWT parsing, signature verification
2. **Token Blacklisting**: Add ability to blacklist revoked tokens
3. **Token Refresh**: Support for refresh token mechanism
4. **Middleware Integration**: Convert to HTTP middleware for automatic validation
5. **Configuration**: Make validation rules configurable (min length, allowed algorithms)

## Dependencies

- userver framework (already used)
- No external JWT libraries needed for dumb validation
- For future real JWT: consider `jwt-cpp` or similar

## Implementation Order

1. Create common library structure and files
2. Update build system
3. Integrate into status_service (has existing validation)
4. Integrate into other services one by one
5. Update tests
6. Create documentation and examples

## Notes

- The "dumb" validation matches existing status_service behavior
- Error responses should match existing format for consistency
- Library is designed to be extended later for real JWT validation
- All services should use the same validation rules for consistency