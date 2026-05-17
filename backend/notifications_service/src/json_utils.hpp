#pragma once

#include <userver/formats/json.hpp>
#include <userver/formats/parse/common_containers.hpp>
#include <userver/formats/serialize/common_containers.hpp>
#include "schemas.hpp"

namespace notifications_service {

namespace json = userver::formats::json;

// Parse V1CurrentUser from JSON
V1CurrentUser Parse(const json::Value& json,
                    userver::formats::parse::To<V1CurrentUser>);

// Parse V1ChannelNotificationNewRequest from JSON
V1ChannelNotificationNewRequest Parse(
    const json::Value& json,
    userver::formats::parse::To<V1ChannelNotificationNewRequest>);

// Parse V1ChannelNotificationListRequest from JSON
V1ChannelNotificationListRequest Parse(
    const json::Value& json,
    userver::formats::parse::To<V1ChannelNotificationListRequest>);

// Parse V1ChannelNotificationReadRequest from JSON
V1ChannelNotificationReadRequest Parse(
    const json::Value& json,
    userver::formats::parse::To<V1ChannelNotificationReadRequest>);

// Serialize V1ChannelNotificationNewResponse to JSON
json::Value Serialize(const V1ChannelNotificationNewResponse& response,
                      userver::formats::serialize::To<json::Value>);

// Serialize V1ChannelNotificationListResponse to JSON
json::Value Serialize(const V1ChannelNotificationListResponse& response,
                      userver::formats::serialize::To<json::Value>);

// Serialize V1ChannelNotificationReadResponse to JSON
json::Value Serialize(const V1ChannelNotificationReadResponse& response,
                      userver::formats::serialize::To<json::Value>);

// Serialize V1NotificationStatus to JSON
json::Value Serialize(const V1NotificationStatus& status,
                      userver::formats::serialize::To<json::Value>);

// Serialize V1Error to JSON
json::Value Serialize(const V1Error& error,
                      userver::formats::serialize::To<json::Value>);

}  // namespace notifications_service