#include "registration_handler.hpp"
#include <userver/formats/json.hpp>
#include <userver/logging/log.hpp>
#include <userver/server/http/http_status.hpp>
#include "json_utils.hpp"

namespace auth_service {

std::string RegistrationHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
  request.GetHttpResponse().SetContentType(
      userver::http::content_type::kApplicationJson);

  try {
    // Parse request body
    auto json_body = userver::formats::json::FromString(request.RequestBody());
    auto registration_request = json_body.As<V1UserRegistrationRequest>();

    // Call business logic
    return HandleRegistration(registration_request);
  } catch (const std::exception& e) {
    LOG_ERROR() << "Registration error: " << e.what();
    request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kBadRequest);
    V1Error error{"validation_error", e.what(), std::nullopt};
    return userver::formats::json::ToString(Serialize(
        error,
        userver::formats::serialize::To<userver::formats::json::Value>{}));
  }
}

std::string RegistrationHandler::HandleRegistration(
    const V1UserRegistrationRequest& request) const {
  // TODO: Implement actual registration logic
  // - Check if user already exists (return 409 if exists)
  // - Hash password
  // - Store user in database
  // - Generate JWT token

  // For now, simulate success
  V1AuthorizedUser authorized_user{
      .login = request.login,
      .name = request.name,
      .token =
          "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9."
          "eyJzdWIiOiJqb2huX2RvZSIsImlhdCI6MTYzOTM... (128 chars)"};
  V1UserAuthorizationResponse response{.current_user = authorized_user};

  LOG_INFO() << "User registered: " << request.login;
  return userver::formats::json::ToString(Serialize(
      response,
      userver::formats::serialize::To<userver::formats::json::Value>{}));
}

}  // namespace auth_service