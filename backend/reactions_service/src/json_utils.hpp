#pragma once

#include <userver/formats/json.hpp>
#include <userver/formats/parse/common_containers.hpp>
#include <userver/formats/serialize/common_containers.hpp>
#include "schemas.hpp"

namespace reactions_service {

namespace json = userver::formats::json;

// Parse V1CurrentUser from JSON
V1CurrentUser Parse(const json::Value& json,
                    userver::formats::parse::To<V1CurrentUser>);

// Parse V1LikeTriggerRequest from JSON
V1LikeTriggerRequest Parse(const json::Value& json,
                           userver::formats::parse::To<V1LikeTriggerRequest>);

// Serialize V1LikeTriggerResponse to JSON
json::Value Serialize(const V1LikeTriggerResponse& response,
                      userver::formats::serialize::To<json::Value>);

// Serialize V1GetReactionsResponse to JSON
json::Value Serialize(const V1GetReactionsResponse& response,
                      userver::formats::serialize::To<json::Value>);

// Serialize V1ReactionEntry to JSON
json::Value Serialize(const V1ReactionEntry& entry,
                      userver::formats::serialize::To<json::Value>);

// Serialize V1Error to JSON
json::Value Serialize(const V1Error& error,
                      userver::formats::serialize::To<json::Value>);

}  // namespace reactions_service