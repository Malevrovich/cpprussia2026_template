#include "notification_read_handler.hpp"

#include <userver/components/component.hpp>
#include <userver/formats/json/exception.hpp>
#include <userver/http/common_headers.hpp>
#include <userver/server/handlers/exceptions.hpp>
#include "../../common/jwt_validation/jwt_validator.hpp"
#include "json_utils.hpp"
#include "notification_storage_component.hpp"

namespace notifications_service {

NotificationReadHandler::NotificationReadHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      storage_(context.FindComponent<NotificationStorageComponent>()) {}

std::string NotificationReadHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
  // Set JSON content type
  request.GetHttpResponse().SetContentType(
      userver::http::content_type::kApplicationJson);

  try {
    // Parse request body
    auto request_body =
        userver::formats::json::FromString(request.RequestBody());
    auto read_request = request_body.As<V1ChannelNotificationReadRequest>();

    return HandleMarkAsRead(request, read_request);
  } catch (const userver::formats::json::MemberMissingException& ex) {
    V1Error error{.error = "INVALID_REQUEST", .message = ex.what()};
    throw userver::server::handlers::ClientError(
        userver::server::handlers::ExternalBody{
            userver::formats::json::ToString(
                Serialize(error, userver::formats::serialize::To<
                                     userver::formats::json::Value>{}))});
  }
}

std::string NotificationReadHandler::HandleMarkAsRead(
    const userver::server::http::HttpRequest& /*http_request*/,
    const V1ChannelNotificationReadRequest& request) const {
  // Validate token using common library
  common::jwt::JwtValidator::ValidateToken(request.current_user.token);

  // Mark notification as read in storage
  bool ok = storage_.MarkNotificationAsRead(
      request.channel_id, request.message_id, request.current_user.login);

  // Prepare response
  V1ChannelNotificationReadResponse response{.ok = ok};

  return userver::formats::json::ToString(Serialize(
      response,
      userver::formats::serialize::To<userver::formats::json::Value>{}));
}

}  // namespace notifications_service