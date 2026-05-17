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
Value Serialize(const ErrorResponse &response,
                userver::formats::serialize::To<Value>) {
  typename Value::Builder builder;
  builder["error"] = response.error;
  builder["message"] = response.message;
  return builder.ExtractValue();
}

} // namespace

void JwtValidator::ValidateToken(const std::string &token) {
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

bool JwtValidator::IsValidToken(const std::string &token) {
  return !token.empty() && token.length() >= kMinTokenLength;
}

std::string JwtValidator::ExtractLoginFromToken(const std::string &token) {
  // Dummy implementation - in real JWT, we would parse the payload
  // For now, return empty string or implement simple extraction if needed
  return "";
}

std::string JwtValidator::ValidateAndExtractLogin(const std::string &token) {
  ValidateToken(token);
  return ExtractLoginFromToken(token);
}

} // namespace common::jwt