#pragma once

#include <string>
#include <userver/server/handlers/exceptions.hpp>

namespace common::jwt {

/// @brief Simple JWT token validator with dumb validation rules
/// @details For now, only validates that token is not empty and has minimum
/// length (128 chars).
///          In the future, this can be extended to perform real JWT validation.
class JwtValidator {
public:
  /// @brief Validates a JWT token
  /// @param token The token to validate
  /// @throws userver::server::handlers::Unauthorized if token is invalid
  static void ValidateToken(const std::string &token);

  /// @brief Checks if a token is valid
  /// @param token The token to check
  /// @return true if token is valid, false otherwise
  static bool IsValidToken(const std::string &token);

  /// @brief Extracts login from token (dummy implementation)
  /// @param token The token to extract login from
  /// @return Extracted login (for now, returns empty string)
  /// @note In a real implementation, this would parse the JWT payload
  static std::string ExtractLoginFromToken(const std::string &token);

  /// @brief Validates token and extracts login
  /// @param token The token to validate and extract from
  /// @return Extracted login if token is valid
  /// @throws userver::server::handlers::Unauthorized if token is invalid
  static std::string ValidateAndExtractLogin(const std::string &token);

private:
  // Minimum token length as per OpenAPI specification
  static constexpr size_t kMinTokenLength = 128;
};

} // namespace common::jwt