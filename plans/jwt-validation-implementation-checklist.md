# JWT Validation Library Implementation Checklist

## Phase 1: Create Common Library Structure

### 1.1 Create Directory Structure
- [ ] Create directory: `backend/common/jwt_validation/`
- [ ] Create root CMakeLists.txt: `backend/common/CMakeLists.txt`
- [ ] Create library CMakeLists.txt: `backend/common/jwt_validation/CMakeLists.txt`

### 1.2 Create Header File (`jwt_validator.hpp`)
```cpp
#pragma once

#include <string>
#include <userver/server/handlers/exceptions.hpp>

namespace common::jwt {

class JwtValidator {
 public:
  static void ValidateToken(const std::string& token);
  static bool IsValidToken(const std::string& token);
  static std::string ExtractLoginFromToken(const std::string& token);
  static std::string ValidateAndExtractLogin(const std::string& token);

 private:
  static constexpr size_t kMinTokenLength = 128;
};

} // namespace common::jwt
```

### 1.3 Create Implementation File (`jwt_validator.cpp`)
```cpp
#include "jwt_validator.hpp"
#include <userver/formats/json.hpp>
#include <userver/formats/serialize/common_containers.hpp>

namespace common::jwt {

namespace {

struct ErrorResponse {
  std::string error;
  std::string message;
};

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
  // Dummy implementation
  return "";
}

std::string JwtValidator::ValidateAndExtractLogin(const std::string& token) {
  ValidateToken(token);
  return ExtractLoginFromToken(token);
}

} // namespace common::jwt
```

### 1.4 Create CMakeLists.txt Files

**`backend/common/CMakeLists.txt`:**
```cmake
cmake_minimum_required(VERSION 3.16)
project(common)

add_subdirectory(jwt_validation)
```

**`backend/common/jwt_validation/CMakeLists.txt`:**
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

## Phase 2: Update Service Build Systems

### 2.1 Update Each Service's CMakeLists.txt
For each service (auth_service, status_service, messaging_service, notifications_service, files_service, reactions_service):

- [ ] Add `add_subdirectory(${CMAKE_SOURCE_DIR}/../common)` before target definition
- [ ] Add `jwt_validation` to `target_link_libraries`

**Example for status_service:**
```cmake
# Add common library
add_subdirectory(${CMAKE_SOURCE_DIR}/../common)

# Later in target_link_libraries:
target_link_libraries(${PROJECT_NAME} PRIVATE
  jwt_validation
  # ... existing dependencies
)
```

## Phase 3: Integrate Library into Services

### 3.1 Auth Service
- [ ] Update `src/authorization_handler.cpp`:
  - Add `#include <common/jwt_validation/jwt_validator.hpp>`
  - In `HandleAuthorization` method, add token validation if needed
- [ ] Update `src/registration_handler.cpp` if it needs token validation

### 3.2 Status Service
- [ ] Update `src/status_update_handler.cpp`:
  - Add `#include <common/jwt_validation/jwt_validator.hpp>`
  - Replace existing `ValidateToken` method call with `common::jwt::JwtValidator::ValidateToken(request.current_user.token)`
  - Remove the old `ValidateToken` method implementation
- [ ] Update `src/status_by_login_handler.cpp`:
  - Add `#include <common/jwt_validation/jwt_validator.hpp>`
  - Add token validation in `HandleRequestThrow` method

### 3.3 Messaging Service
- [ ] Update `src/message_new_handler.cpp`:
  - Add `#include <common/jwt_validation/jwt_validator.hpp>`
  - In `HandleNewMessage` method, add `common::jwt::JwtValidator::ValidateToken(request.current_user.token)` after parsing request
- [ ] Update `src/message_by_timestamp_handler.cpp` (if exists):
  - Add token validation

### 3.4 Notifications Service
- [ ] Update `src/notification_new_handler.cpp`:
  - Add `#include <common/jwt_validation/jwt_validator.hpp>`
  - Add token validation
- [ ] Update `src/notification_list_handler.cpp`:
  - Add `#include <common/jwt_validation/jwt_validator.hpp>`
  - Add token validation

### 3.5 Files Service
- [ ] Update `src/file_new_handler.cpp`:
  - Add `#include <common/jwt_validation/jwt_validator.hpp>`
  - Add token validation
- [ ] Update `src/file_by_uri_handler.cpp`:
  - Add `#include <common/jwt_validation/jwt_validator.hpp>`
  - Add token validation

### 3.6 Reactions Service
- [ ] Update `src/get_reactions_handler.cpp`:
  - Add `#include <common/jwt_validation/jwt_validator.hpp>`
  - Add token validation
- [ ] Update `src/like_trigger_handler.cpp`:
  - Add `#include <common/jwt_validation/jwt_validator.hpp>`
  - Add token validation

## Phase 4: Testing

### 4.1 Update Existing Tests
- [ ] Update status_service tests to work with new validation
- [ ] Update other service tests to include token validation

### 4.2 Test Scenarios
- [ ] Test with valid token (128+ characters)
- [ ] Test with empty token (should return 401)
- [ ] Test with short token (< 128 characters, should return 401)
- [ ] Test with valid token in all services

### 4.3 Integration Testing
- [ ] Build all services to ensure no compilation errors
- [ ] Run existing test suites
- [ ] Test inter-service communication with tokens

## Phase 5: Documentation

### 5.1 Create Usage Examples
- [ ] Create `backend/common/jwt_validation/README.md` with usage examples
- [ ] Update service README files to mention token validation

### 5.2 API Documentation
- [ ] Document the JwtValidator class interface
- [ ] Document error responses
- [ ] Document future extension points

## Implementation Notes

1. **Error Response Consistency**: Ensure all services return the same error format:
   ```json
   {"error": "unauthorized", "message": "Token is required"}
   ```

2. **Header Inclusion**: Always include the header as:
   ```cpp
   #include <common/jwt_validation/jwt_validator.hpp>
   ```

3. **Namespace Usage**: Use `common::jwt::JwtValidator::ValidateToken(token)`

4. **Exception Handling**: The library throws `userver::server::handlers::Unauthorized` - services should let this propagate or catch and rethrow as appropriate.

5. **Build Order**: Build common library first, then services.

## Verification Checklist

- [ ] All services compile without errors
- [ ] All existing tests pass
- [ ] Token validation works correctly in all services
- [ ] Error responses are consistent across services
- [ ] No duplicate validation code exists in services
- [ ] CMake dependencies are correctly set up
- [ ] Library can be extended in the future for real JWT validation

## Future Enhancement Notes

1. **Real JWT Validation**: Replace dumb validation with real JWT parsing
2. **Configuration**: Make minimum token length configurable
3. **Token Blacklisting**: Add ability to check against blacklist
4. **Middleware**: Convert to HTTP middleware for automatic validation
5. **Performance**: Add caching for validated tokens