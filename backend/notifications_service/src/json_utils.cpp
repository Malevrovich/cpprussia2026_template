#include "json_utils.hpp"

namespace notifications_service {

V1CurrentUser Parse(const json::Value& json,
                    userver::formats::parse::To<V1CurrentUser>) {
  V1CurrentUser result;
  result.token = json["token"].As<std::string>("");
  result.login = json["login"].As<std::string>();
  result.name = json["name"].As<std::string>();
  return result;
}

V1ChannelNotificationNewRequest Parse(
    const json::Value& json,
    userver::formats::parse::To<V1ChannelNotificationNewRequest>) {
  V1ChannelNotificationNewRequest result;
  result.current_user = json["current_user"].As<V1CurrentUser>();
  result.channel_id = json["channel_id"].As<V1ChannelId>();
  result.message_id = json["message_id"].As<V1MessageId>();
  result.other_user_login = json["other_user_login"].As<V1Login>();
  return result;
}

V1ChannelNotificationListRequest Parse(
    const json::Value& json,
    userver::formats::parse::To<V1ChannelNotificationListRequest>) {
  V1ChannelNotificationListRequest result;
  result.current_user = json["current_user"].As<V1CurrentUser>();
  result.channel_id = json["channel_id"].As<V1ChannelId>();
  return result;
}

json::Value Serialize(const V1ChannelNotificationNewResponse& response,
                      userver::formats::serialize::To<json::Value>) {
  json::ValueBuilder builder;
  builder["notification_id"] = response.notification_id;
  return builder.ExtractValue();
}

json::Value Serialize(const V1ChannelNotificationListResponse& response,
                      userver::formats::serialize::To<json::Value>) {
  json::ValueBuilder builder;
  builder["notifications"] = response.notifications;
  return builder.ExtractValue();
}

json::Value Serialize(const V1NotificationStatus& status,
                      userver::formats::serialize::To<json::Value>) {
  json::ValueBuilder builder;
  builder["message_id"] = status.message_id;
  builder["read"] = status.read;
  return builder.ExtractValue();
}

json::Value Serialize(const V1Error& error,
                      userver::formats::serialize::To<json::Value>) {
  json::ValueBuilder builder;
  builder["error"] = error.error;
  builder["code"] = error.code;
  return builder.ExtractValue();
}

}  // namespace notifications_service