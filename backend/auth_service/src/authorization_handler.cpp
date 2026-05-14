#include "authorization_handler.hpp"
#include <userver/formats/json.hpp>
#include <userver/logging/log.hpp>
#include <userver/server/http/http_status.hpp>
#include "json_utils.hpp"

namespace auth_service {

std::string AuthorizationHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
  request.GetHttpResponse().SetContentType(
      userver::http::content_type::kApplicationJson);

  try {
    // Parse request body
    auto json_body = userver::formats::json::FromString(request.RequestBody());
    auto auth_request = json_body.As<V1UserAuthorizationRequest>();

    // Call business logic
    return HandleAuthorization(auth_request);
  } catch (const std::exception& e) {
    LOG_ERROR() << "Authorization error: " << e.what();
    request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kBadRequest);
    V1Error error{"validation_error", e.what(), std::nullopt};
    return userver::formats::json::ToString(Serialize(
        error,
        userver::formats::serialize::To<userver::formats::json::Value>{}));
  }
}

std::string AuthorizationHandler::HandleAuthorization(
    const V1UserAuthorizationRequest& request) const {
  // TODO: Implement actual authorization logic
  // - Look up user by login
  // - Verify password hash
  // - If invalid, return 401
  // - Generate JWT token

  // For now, simulate success
  V1AuthorizedUser authorized_user{
      .login = request.login,
      .name = "John Doe",  // Should be fetched from DB
      .token =
          "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9."
          "eyJzdWIiOiJqb2huX2RvZSIsImlhdCI6MTYzOTM... (128 chars)"};
  V1UserAuthorizationResponse response{.current_user = authorized_user};

  LOG_INFO() << "User authorized: " << request.login;
  return userver::formats::json::ToString(Serialize(
      response,
      userver::formats::serialize::To<userver::formats::json::Value>{}));
}

}  // namespace auth_service