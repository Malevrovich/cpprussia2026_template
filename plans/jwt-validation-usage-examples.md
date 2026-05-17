# JWT Validation Library Usage Examples

## Overview
The JWT validation library provides a simple interface for validating JWT tokens across all microservices. Currently implements "dumb" validation (checking token length and non-emptiness).

## Quick Start

### 1. Include the Header
```cpp
#include <common/jwt_validation/jwt_validator.hpp>
```

### 2. Basic Token Validation
```cpp
// In your HTTP handler's HandleRequestThrow method:
try {
  common::jwt::JwtValidator::ValidateToken(request.current_user.token);
  // Token is valid, proceed with request processing
} catch (const userver::server::handlers::Unauthorized& e) {
  // Exception will automatically return 401 Unauthorized
  throw;
}
```

### 3. Check Token Validity (Non-throwing)
```cpp
if (common::jwt::JwtValidator::IsValidToken(token)) {
  // Token is valid
} else {
  // Token is invalid
}
```

## Complete Integration Example

### Before Integration (status_service example):
```cpp
// Old method in status_update_handler.cpp
void StatusUpdateHandler::ValidateToken(const std::string& token) const {
  if (token.empty()) {
    throw userver::server::handlers::Unauthorized(...);
  }
  if (token.length() < 128) {
    throw userver::server::handlers::Unauthorized(...);
  }
}

// Usage in HandleRequestThrow
ValidateToken(request.current_user.token);
```

### After Integration:
```cpp
#include <common/jwt_validation/jwt_validator.hpp>

// Remove the old ValidateToken method entirely

// In HandleRequestThrow:
common::jwt::JwtValidator::ValidateToken(request.current_user.token);
```

## Service-Specific Examples

### Auth Service
```cpp
// In authorization_handler.cpp
std::string AuthorizationHandler::HandleAuthorization(
    const userver::server::http::HttpRequest& http_request,
    const V1UserAuthorizationRequest& request) const {
  
  // Validate credentials and generate token
  // ... existing auth logic ...
  
  // For future endpoints that require token validation:
  // common::jwt::JwtValidator::ValidateToken(incoming_token);
  
  return response;
}
```

### Messaging Service
```cpp
// In message_new_handler.cpp
std::string MessageNewHandler::HandleNewMessage(
    const userver::server::http::HttpRequest& http_request,
    const V1ChannelMessageNewRequest& request) const {
  
  // Validate token first
  common::jwt::JwtValidator::ValidateToken(request.current_user.token);
  
  // Then validate other fields
  if (request.message.empty()) {
    // ... error handling ...
  }
  
  // Process message
  // ...
}
```

### Files Service
```cpp
// In file_new_handler.cpp
std::string FileNewHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& http_request,
    userver::server::request::RequestContext&) const {
  
  // Parse request
  auto request_body = http_request.RequestBody();
  auto json = userver::formats::json::FromString(request_body);
  auto request = json.As<V1FileNewRequest>();
  
  // Validate token
  common::jwt::JwtValidator::ValidateToken(request.current_user.token);
  
  // Process file upload
  // ...
}
```

## Error Handling

### Expected Error Responses
The library throws `userver::server::handlers::Unauthorized` with JSON body:

1. **Empty token**:
```json
{"error": "unauthorized", "message": "Token is required"}
```

2. **Token too short** (< 128 characters):
```json
{"error": "unauthorized", "message": "Invalid token format"}
```

### Custom Error Handling
```cpp
try {
  common::jwt::JwtValidator::ValidateToken(token);
} catch (const userver::server::handlers::Unauthorized& e) {
  // Log the error
  LOG_WARNING() << "Token validation failed: " << e.what();
  
  // You can rethrow with custom message or handle differently
  throw userver::server::handlers::Unauthorized(
      userver::server::handlers::ExternalBody{
          userver::formats::json::ToString(
              Serialize(CustomErrorResponse{"auth_failed", "Invalid credentials"},
                        userver::formats::serialize::To<
                            userver::formats::json::Value>{}))});
}
```

## Testing Examples

### Unit Test Example
```cpp
#include <common/jwt_validation/jwt_validator.hpp>
#include <userver/utest/utest.hpp>

UTEST(JwtValidatorTest, ValidToken) {
  // Valid token (128 characters)
  std::string valid_token(128, 'a');
  EXPECT_NO_THROW(common::jwt::JwtValidator::ValidateToken(valid_token));
  EXPECT_TRUE(common::jwt::JwtValidator::IsValidToken(valid_token));
}

UTEST(JwtValidatorTest, EmptyToken) {
  std::string empty_token;
  EXPECT_THROW(common::jwt::JwtValidator::ValidateToken(empty_token),
               userver::server::handlers::Unauthorized);
  EXPECT_FALSE(common::jwt::JwtValidator::IsValidToken(empty_token));
}

UTEST(JwtValidatorTest, ShortToken) {
  std::string short_token(127, 'a');  // 127 characters
  EXPECT_THROW(common::jwt::JwtValidator::ValidateToken(short_token),
               userver::server::handlers::Unauthorized);
  EXPECT_FALSE(common::jwt::JwtValidator::IsValidToken(short_token));
}
```

### Integration Test Example (Python pytest)
```python
import pytest

async def test_status_update_with_valid_token(service_client):
    # Valid token (128 characters)
    valid_token = 'a' * 128
    
    response = await service_client.post(
        '/v1/user/status/update',
        json={
            'current_user': {
                'token': valid_token,
                'login': 'testuser',
                'name': 'Test User'
            },
            'status': {
                'status_type': 'kOnline',
                'status_message': 'Available',
                'visibility': 'kPublic'
            }
        }
    )
    assert response.status == 200

async def test_status_update_with_invalid_token(service_client):
    # Short token (127 characters)
    short_token = 'a' * 127
    
    response = await service_client.post(
        '/v1/user/status/update',
        json={
            'current_user': {
                'token': short_token,
                'login': 'testuser',
                'name': 'Test User'
            },
            'status': {
                'status_type': 'kOnline',
                'status_message': 'Available',
                'visibility': 'kPublic'
            }
        }
    )
    assert response.status == 401
    assert response.json() == {
        'error': 'unauthorized',
        'message': 'Invalid token format'
    }
```

## Build Configuration

### CMake Configuration
Each service's CMakeLists.txt should include:

```cmake
# Add common library
add_subdirectory(${CMAKE_SOURCE_DIR}/../common)

# Link against jwt_validation
target_link_libraries(your_service_name PRIVATE
  jwt_validation
  # ... other dependencies
)
```

### Dependencies
The library depends on:
- `userver::core`
- `userver::server` 
- `userver::formats_json`

These are automatically linked when using `target_link_libraries(jwt_validation ...)`.

## Future Extensions

### 1. Real JWT Validation
When ready to implement real JWT validation:

```cpp
// Future implementation
std::string JwtValidator::ExtractLoginFromToken(const std::string& token) {
  // Parse JWT token
  auto decoded = jwt::decode(token);
  auto payload = decoded.get_payload();
  
  // Extract login from payload
  return payload["login"].as_string();
}
```

### 2. Configuration
Make validation rules configurable:

```cpp
class JwtValidator {
 public:
  static void Configure(size_t min_token_length, bool require_signature);
  // ...
};
```

### 3. Middleware Integration
Convert to HTTP middleware for automatic validation:

```cpp
class JwtValidationMiddleware : public server::http::HttpMiddlewareBase {
  // Automatically validates token for all requests
};
```

## Best Practices

1. **Always validate tokens early** in request processing
2. **Use the library consistently** across all services
3. **Let exceptions propagate** unless you need custom error handling
4. **Update tests** to include token validation scenarios
5. **Monitor authentication failures** in logs

## Troubleshooting

### Common Issues

1. **Compilation error: "common/jwt_validation/jwt_validator.hpp not found"**
   - Ensure `add_subdirectory(${CMAKE_SOURCE_DIR}/../common)` is called
   - Check include paths in CMakeLists.txt

2. **Linker error: undefined reference to JwtValidator methods**
   - Ensure `jwt_validation` is in `target_link_libraries`
   - Rebuild the common library first

3. **401 errors even with valid tokens**
   - Verify token length is exactly 128+ characters
   - Check token is not empty or contains only whitespace

4. **Inconsistent error responses**
   - Ensure all services use `common::jwt::JwtValidator::ValidateToken()`
   - Don't mix with custom validation code

### Debugging
```cpp
// Add debug logging
LOG_DEBUG() << "Validating token, length: " << token.length();
common::jwt::JwtValidator::ValidateToken(token);
LOG_DEBUG() << "Token validation passed";
```

## Support
For issues or questions:
1. Check the library implementation in `backend/common/jwt_validation/`
2. Review service integration examples
3. Consult the comprehensive test suite