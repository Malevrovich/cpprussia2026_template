#include "json_utils.hpp"

#include <userver/formats/json.hpp>
#include <userver/logging/log.hpp>

namespace reactions_service {

namespace json = userver::formats::json;

// Helper function to convert V1Animation to string
std::string ToString(V1Animation animation) {
  switch (animation) {
    case V1Animation::kLike:
      return "like";
    case V1Animation::kDislike:
      return "dislike";
    case V1Animation::kHeart:
      return "heart";
    case V1Animation::kFire:
      return "fire";
    case V1Animation::kOkay:
      return "okay";
    case V1Animation::kLol:
      return "LOL";
    case V1Animation::kSmile:
      return "smile";
    default:
      throw std::runtime_error("Unknown animation type");
  }
}

// Helper function to convert string to V1Animation
V1Animation AnimationFromString(const std::string& str) {
  if (str == "like") return V1Animation::kLike;
  if (str == "dislike") return V1Animation::kDislike;
  if (str == "heart") return V1Animation::kHeart;
  if (str == "fire") return V1Animation::kFire;
  if (str == "okay") return V1Animation::kOkay;
  if (str == "LOL") return V1Animation::kLol;
  if (str == "smile") return V1Animation::kSmile;
  throw std::runtime_error("Invalid animation: " + str);
}

// Parse V1CurrentUser from JSON
V1CurrentUser Parse(const json::Value& json,
                    userver::formats::parse::To<V1CurrentUser>) {
  V1CurrentUser result;
  result.token = json["token"].As<std::string>();
  result.login = json["login"].As<std::string>();
  result.name = json["name"].As<std::string>();
  return result;
}

// Parse V1LikeTriggerRequest from JSON
V1LikeTriggerRequest Parse(const json::Value& json,
                           userver::formats::parse::To<V1LikeTriggerRequest>) {
  V1LikeTriggerRequest result;
  result.current_user = json["current_user"].As<V1CurrentUser>();
  result.idempotency_token = json["idempotency_token"].As<std::string>();
  result.channel_id = json["channel_id"].As<V1ChannelId>();
  result.message_id = json["message_id"].As<V1MessageId>();
  result.animation = AnimationFromString(json["animation"].As<std::string>());
  return result;
}

// Serialize V1LikeTriggerResponse to JSON
json::Value Serialize(const V1LikeTriggerResponse& response,
                      userver::formats::serialize::To<json::Value>) {
  json::ValueBuilder builder;
  builder["action"] =
      (response.action == V1LikeTriggerResponse::Action::kAdded ? "added"
                                                                : "removed");
  if (response.current_user_reaction.has_value()) {
    builder["current_user_reaction"] =
        ToString(response.current_user_reaction.value());
  } else {
    builder["current_user_reaction"] = json::Value();
  }
  return builder.ExtractValue();
}

// Serialize V1ReactionEntry to JSON
json::Value Serialize(const V1ReactionEntry& entry,
                      userver::formats::serialize::To<json::Value>) {
  json::ValueBuilder builder;
  builder["user"] = entry.user;
  builder["animation"] = ToString(entry.animation);
  return builder.ExtractValue();
}

// Serialize V1GetReactionsResponse to JSON
json::Value Serialize(const V1GetReactionsResponse& response,
                      userver::formats::serialize::To<json::Value>) {
  json::ValueBuilder builder;
  json::ValueBuilder reactions_builder(json::Type::kArray);
  for (const auto& reaction : response.reactions) {
    reactions_builder.PushBack(
        Serialize(reaction, userver::formats::serialize::To<json::Value>{}));
  }
  builder["reactions"] = reactions_builder.ExtractValue();
  return builder.ExtractValue();
}

// Serialize V1Error to JSON
json::Value Serialize(const V1Error& error,
                      userver::formats::serialize::To<json::Value>) {
  json::ValueBuilder builder;
  builder["code"] = error.code;
  builder["message"] = error.message;
  return builder.ExtractValue();
}

}  // namespace reactions_service