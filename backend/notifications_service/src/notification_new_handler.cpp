#include "notification_new_handler.hpp"

#include <userver/components/component.hpp>
#include <userver/formats/json/exception.hpp>
#include <userver/http/common_headers.hpp>
#include <userver/server/handlers/exceptions.hpp>
#include "../../common/jwt_validation/jwt_validator.hpp"
#include "json_utils.hpp"
#include "notification_storage_component.hpp"

namespace notifications_service {

NotificationNewHandler::NotificationNewHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      storage_(context.FindComponent<NotificationStorageComponent>()) {}

std::string NotificationNewHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
  // Set JSON content type
  request.GetHttpResponse().SetContentType(
      userver::http::content_type::kApplicationJson);

  try {
    // Parse request body
    auto request_body =
        userver::formats::json::FromString(request.RequestBody());
    auto create_request = request_body.As<V1ChannelNotificationNewRequest>();

    return HandleCreateNotification(request, create_request);
  } catch (const userver::formats::json::MemberMissingException& ex) {
    V1Error error{.error = "INVALID_REQUEST", .code = 400};
    throw userver::server::handlers::ClientError(
        userver::server::handlers::ExternalBody{
            userver::formats::json::ToString(
                Serialize(error, userver::formats::serialize::To<
                                     userver::formats::json::Value>{}))});
  }
}

std::string NotificationNewHandler::HandleCreateNotification(
    const userver::server::http::HttpRequest& /*http_request*/,
    const V1ChannelNotificationNewRequest& request) const {
  // Basic validation
  if (request.other_user_login.empty()) {
    V1Error error{.error = "INVALID_REQUEST", .code = 400};
    throw userver::server::handlers::ClientError(
        userver::server::handlers::ExternalBody{
            userver::formats::json::ToString(
                Serialize(error, userver::formats::serialize::To<
                                     userver::formats::json::Value>{}))});
  }

  // Validate token using common library
  common::jwt::JwtValidator::ValidateToken(request.current_user.token);

  // Create notification in storage
  std::string notification_id = storage_.CreateNotification(
      request.channel_id, request.message_id, request.other_user_login,
      request.current_user.login);

  // Prepare response
  V1ChannelNotificationNewResponse response{.notification_id = notification_id};

  return userver::formats::json::ToString(Serialize(
      response,
      userver::formats::serialize::To<userver::formats::json::Value>{}));
}

}  // namespace notifications_service