#include "notification_list_handler.hpp"

#include <userver/components/component.hpp>
#include <userver/formats/json/exception.hpp>
#include <userver/http/common_headers.hpp>
#include <userver/server/handlers/exceptions.hpp>
#include "../../common/jwt_validation/jwt_validator.hpp"
#include "json_utils.hpp"
#include "notification_storage_component.hpp"

namespace notifications_service {

NotificationListHandler::NotificationListHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      storage_(context.FindComponent<NotificationStorageComponent>()) {}

std::string NotificationListHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
  // Set JSON content type
  request.GetHttpResponse().SetContentType(
      userver::http::content_type::kApplicationJson);

  try {
    // Parse request body
    auto request_body =
        userver::formats::json::FromString(request.RequestBody());
    auto list_request = request_body.As<V1ChannelNotificationListRequest>();

    return HandleListNotifications(request, list_request);
  } catch (const userver::formats::json::MemberMissingException& ex) {
    V1Error error{.error = "INVALID_REQUEST", .code = 400};
    throw userver::server::handlers::ClientError(
        userver::server::handlers::ExternalBody{
            userver::formats::json::ToString(
                Serialize(error, userver::formats::serialize::To<
                                     userver::formats::json::Value>{}))});
  }
}

std::string NotificationListHandler::HandleListNotifications(
    const userver::server::http::HttpRequest& /*http_request*/,
    const V1ChannelNotificationListRequest& request) const {
  // Validate token using common library
  common::jwt::JwtValidator::ValidateToken(request.current_user.token);

  // Get notifications from storage
  auto notifications = storage_.GetUserNotifications(
      request.channel_id, request.current_user.login);

  // Prepare response
  V1ChannelNotificationListResponse response{.notifications =
                                                 std::move(notifications)};

  return userver::formats::json::ToString(Serialize(
      response,
      userver::formats::serialize::To<userver::formats::json::Value>{}));
}

}  // namespace notifications_service