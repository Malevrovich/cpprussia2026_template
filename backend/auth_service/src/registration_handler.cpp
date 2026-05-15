#include "registration_handler.hpp"
#include <userver/components/component_context.hpp>
#include <userver/formats/json.hpp>
#include <userver/logging/log.hpp>
#include <userver/server/http/http_status.hpp>
#include "crypto_utils.hpp"
#include "json_utils.hpp"
#include "user_storage_component.hpp"

namespace auth_service {

RegistrationHandler::RegistrationHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      storage_(context.FindComponent<UserStorageComponent>()) {}

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
    return HandleRegistration(request, registration_request);
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
    const userver::server::http::HttpRequest& http_request,
    const V1UserRegistrationRequest& request) const {
  // Check if user already exists
  if (storage_.FindUser(request.login).has_value()) {
    LOG_WARNING() << "User already exists: " << request.login;
    http_request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kConflict);
    V1Error error{"USER_ALREADY_EXISTS", "User with this login already exists",
                  std::nullopt};
    return userver::formats::json::ToString(Serialize(
        error,
        userver::formats::serialize::To<userver::formats::json::Value>{}));
  }

  // Generate salt and hash password
  std::string salt = GenerateSalt();
  std::string password_hash = HashPassword(request.password, salt);

  // Create user object
  User user{
      .login = request.login,
      .name = request.name,
      .email = request.email,
      .phone = request.phone,
      .password_hash = password_hash,
      .salt = salt,
  };

  // Store user
  if (!storage_.AddUser(user)) {
    // This should not happen because we already checked, but just in case
    http_request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kConflict);
    V1Error error{"USER_ALREADY_EXISTS", "User with this login already exists",
                  std::nullopt};
    return userver::formats::json::ToString(Serialize(
        error,
        userver::formats::serialize::To<userver::formats::json::Value>{}));
  }

  // Generate token
  std::string token = storage_.GenerateToken();

  // Build response
  V1AuthorizedUser authorized_user{
      .login = request.login,
      .name = request.name,
      .token = token,
  };
  V1UserAuthorizationResponse response{.current_user = authorized_user};

  LOG_INFO() << "User registered: " << request.login;
  http_request.GetHttpResponse().SetStatus(
      userver::server::http::HttpStatus::kOk);
  return userver::formats::json::ToString(Serialize(
      response,
      userver::formats::serialize::To<userver::formats::json::Value>{}));
}

}  // namespace auth_service