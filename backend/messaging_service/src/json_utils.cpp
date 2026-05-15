#include "json_utils.hpp"
#include <userver/utils/datetime.hpp>

namespace messaging_service {

V1CurrentUser Parse(const json::Value& json,
                    userver::formats::parse::To<V1CurrentUser>) {
  V1CurrentUser result;
  result.token = json["token"].As<std::string>("");
  result.login = json["login"].As<std::string>();
  result.name = json["name"].As<std::string>();
  return result;
}

V1ChannelMessageNewRequest Parse(
    const json::Value& json,
    userver::formats::parse::To<V1ChannelMessageNewRequest>) {
  V1ChannelMessageNewRequest result;
  result.current_user = json["current_user"].As<V1CurrentUser>();
  result.channel_id = json["channel_id"].As<V1ChannelId>();
  result.message = json["message"].As<std::string>();
  return result;
}

V1ChannelMessageByTimestampRequest Parse(
    const json::Value& json,
    userver::formats::parse::To<V1ChannelMessageByTimestampRequest>) {
  V1ChannelMessageByTimestampRequest result;
  result.channel_id = json["channel_id"].As<V1ChannelId>();
  result.from = json["from"].As<std::string>();

  if (json.HasMember("to")) {
    result.to = json["to"].As<std::string>();
  }

  if (json.HasMember("limit")) {
    result.limit = json["limit"].As<int32_t>();
  }

  return result;
}

json::Value Serialize(const V1ChannelMessageNewResponse& response,
                      userver::formats::serialize::To<json::Value>) {
  json::ValueBuilder builder;
  builder["message_id"] = response.message_id;
  return builder.ExtractValue();
}

json::Value Serialize(const V1ChannelMessageByTimestampResponse& response,
                      userver::formats::serialize::To<json::Value>) {
  json::ValueBuilder builder;

  json::ValueBuilder messages_builder(json::Type::kArray);
  for (const auto& message : response.messages) {
    messages_builder.PushBack(
        Serialize(message, userver::formats::serialize::To<json::Value>{}));
  }
  builder["messages"] = messages_builder.ExtractValue();

  if (response.next_cursor.has_value()) {
    builder["next_cursor"] = response.next_cursor.value();
  } else {
    builder["next_cursor"] = json::Value();
  }

  builder["has_more"] = response.has_more;

  return builder.ExtractValue();
}

json::Value Serialize(const V1ChannelMessage& message,
                      userver::formats::serialize::To<json::Value>) {
  json::ValueBuilder builder;

  json::ValueBuilder user_builder;
  user_builder["token"] = message.current_user.token;
  user_builder["login"] = message.current_user.login;
  user_builder["name"] = message.current_user.name;
  builder["current_user"] = user_builder.ExtractValue();

  builder["id"] = message.id;
  builder["timestamp"] = message.timestamp;
  builder["message"] = message.message;

  return builder.ExtractValue();
}

json::Value Serialize(const V1Error& error,
                      userver::formats::serialize::To<json::Value>) {
  json::ValueBuilder builder;
  builder["error"] = error.error;
  builder["code"] = error.code;
  return builder.ExtractValue();
}

}  // namespace messaging_service