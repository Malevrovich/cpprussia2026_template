#include "json_utils.hpp"
#include <userver/formats/json.hpp>
#include <userver/utils/datetime.hpp>

namespace status_service {

namespace {

// Static helper functions (internal to this translation unit)
static std::string StatusTypeToStringImpl(V1StatusType status_type) {
  switch (status_type) {
    case V1StatusType::kOnline:
      return "online";
    case V1StatusType::kAway:
      return "away";
    case V1StatusType::kBusy:
      return "busy";
    case V1StatusType::kOffline:
      return "offline";
    default:
      return "offline";
  }
}

static std::string VisibilityToStringImpl(V1Visibility visibility) {
  switch (visibility) {
    case V1Visibility::kPublic:
      return "public";
    case V1Visibility::kPrivate:
      return "private";
    default:
      return "public";
  }
}

static V1StatusType StringToStatusTypeImpl(const std::string& status_type) {
  if (status_type == "online") return V1StatusType::kOnline;
  if (status_type == "away") return V1StatusType::kAway;
  if (status_type == "busy") return V1StatusType::kBusy;
  if (status_type == "offline") return V1StatusType::kOffline;
  throw std::runtime_error("Invalid status type: " + status_type);
}

static V1Visibility StringToVisibilityImpl(const std::string& visibility) {
  if (visibility == "public") return V1Visibility::kPublic;
  if (visibility == "private") return V1Visibility::kPrivate;
  throw std::runtime_error("Invalid visibility: " + visibility);
}

}  // namespace

V1CurrentUser Parse(const json::Value& json,
                    userver::formats::parse::To<V1CurrentUser>) {
  V1CurrentUser user;
  user.token = json["token"].As<std::string>("");
  user.login = json["login"].As<std::string>();
  user.name = json["name"].As<std::string>("");
  return user;
}

V1UserStatus Parse(const json::Value& json,
                   userver::formats::parse::To<V1UserStatus>) {
  V1UserStatus status;
  status.status_type =
      StringToStatusTypeImpl(json["status_type"].As<std::string>());
  status.status_message = json["status_message"].As<std::string>();

  if (json.HasMember("visibility")) {
    status.visibility =
        StringToVisibilityImpl(json["visibility"].As<std::string>());
  }

  return status;
}

V1UserStatusUpdateRequest Parse(
    const json::Value& json,
    userver::formats::parse::To<V1UserStatusUpdateRequest>) {
  V1UserStatusUpdateRequest request;
  request.current_user = json["current_user"].As<V1CurrentUser>();
  request.status = json["status"].As<V1UserStatus>();
  return request;
}

V1UserStatusByLoginRequest Parse(
    const json::Value& json,
    userver::formats::parse::To<V1UserStatusByLoginRequest>) {
  V1UserStatusByLoginRequest request;
  request.current_user = json["current_user"].As<V1CurrentUser>();
  request.login = json["login"].As<std::string>();
  return request;
}

json::Value Serialize(const V1UserStatusUpdateResponse& response,
                      userver::formats::serialize::To<json::Value>) {
  json::ValueBuilder builder;
  builder["success"] = response.success;
  builder["updated_at"] =
      userver::utils::datetime::Timestring(response.updated_at);

  if (response.expires_at.has_value()) {
    builder["expires_at"] =
        userver::utils::datetime::Timestring(*response.expires_at);
  }

  return builder.ExtractValue();
}

json::Value Serialize(const V1UserStatusByLoginResponse& response,
                      userver::formats::serialize::To<json::Value>) {
  json::ValueBuilder builder;
  builder["status"] = Serialize(response.status,
                                userver::formats::serialize::To<json::Value>{});
  builder["updated_at"] =
      userver::utils::datetime::Timestring(response.updated_at);
  return builder.ExtractValue();
}

json::Value Serialize(const V1ErrorResponse& error,
                      userver::formats::serialize::To<json::Value>) {
  json::ValueBuilder builder;
  builder["error"] = error.error;
  builder["message"] = error.message;
  return builder.ExtractValue();
}

json::Value Serialize(const V1UserStatus& status,
                      userver::formats::serialize::To<json::Value>) {
  json::ValueBuilder builder;
  builder["status_type"] = StatusTypeToStringImpl(status.status_type);
  builder["status_message"] = status.status_message;
  builder["visibility"] = VisibilityToStringImpl(status.visibility);
  return builder.ExtractValue();
}

}  // namespace status_service