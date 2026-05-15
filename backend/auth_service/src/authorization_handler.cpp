#include "authorization_handler.hpp"
#include <userver/components/component_context.hpp>
#include <userver/formats/json.hpp>
#include <userver/logging/log.hpp>
#include <userver/server/http/http_status.hpp>
#include "json_utils.hpp"
#include "user_storage_component.hpp"

namespace auth_service {

AuthorizationHandler::AuthorizationHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      storage_(context.FindComponent<UserStorageComponent>()) {}

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
    return HandleAuthorization(request, auth_request);
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
    const userver::server::http::HttpRequest& http_request,
    const V1UserAuthorizationRequest& request) const {
  // Look up user
  auto user_opt = storage_.FindUser(request.login);
  if (!user_opt.has_value()) {
    LOG_WARNING() << "User not found: " << request.login;
    http_request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kUnauthorized);
    V1Error error{"INVALID_CREDENTIALS", "Invalid login or password",
                  std::nullopt};
    return userver::formats::json::ToString(Serialize(
        error,
        userver::formats::serialize::To<userver::formats::json::Value>{}));
  }

  // Verify password
  if (!storage_.VerifyPassword(request.login, request.password)) {
    LOG_WARNING() << "Invalid password for user: " << request.login;
    http_request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kUnauthorized);
    V1Error error{"INVALID_CREDENTIALS", "Invalid login or password",
                  std::nullopt};
    return userver::formats::json::ToString(Serialize(
        error,
        userver::formats::serialize::To<userver::formats::json::Value>{}));
  }

  // Generate token
  std::string token = storage_.GenerateToken();

  // Build response
  const User& user = user_opt.value();
  V1AuthorizedUser authorized_user{
      .login = user.login,
      .name = user.name,
      .token = token,
  };
  V1UserAuthorizationResponse response{.current_user = authorized_user};

  LOG_INFO() << "User authorized: " << request.login;
  http_request.GetHttpResponse().SetStatus(
      userver::server::http::HttpStatus::kOk);
  return userver::formats::json::ToString(Serialize(
      response,
      userver::formats::serialize::To<userver::formats::json::Value>{}));
}

}  // namespace auth_service