#pragma once

#include <userver/formats/json.hpp>
#include <userver/formats/parse/common_containers.hpp>
#include <userver/formats/serialize/common_containers.hpp>
#include "schemas.hpp"

namespace messaging_service {

namespace json = userver::formats::json;

// Parse V1CurrentUser from JSON
V1CurrentUser Parse(const json::Value& json,
                    userver::formats::parse::To<V1CurrentUser>);

// Parse V1ChannelMessageNewRequest from JSON
V1ChannelMessageNewRequest Parse(
    const json::Value& json,
    userver::formats::parse::To<V1ChannelMessageNewRequest>);

// Parse V1ChannelMessageByTimestampRequest from JSON
V1ChannelMessageByTimestampRequest Parse(
    const json::Value& json,
    userver::formats::parse::To<V1ChannelMessageByTimestampRequest>);

// Serialize V1ChannelMessageNewResponse to JSON
json::Value Serialize(const V1ChannelMessageNewResponse& response,
                      userver::formats::serialize::To<json::Value>);

// Serialize V1ChannelMessageByTimestampResponse to JSON
json::Value Serialize(const V1ChannelMessageByTimestampResponse& response,
                      userver::formats::serialize::To<json::Value>);

// Serialize V1ChannelMessage to JSON
json::Value Serialize(const V1ChannelMessage& message,
                      userver::formats::serialize::To<json::Value>);

// Serialize V1Error to JSON
json::Value Serialize(const V1Error& error,
                      userver::formats::serialize::To<json::Value>);

}  // namespace messaging_service