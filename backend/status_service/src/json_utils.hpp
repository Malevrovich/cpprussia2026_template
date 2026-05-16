#pragma once

#include <userver/formats/json.hpp>
#include <userver/formats/parse/common_containers.hpp>
#include <userver/formats/serialize/common_containers.hpp>
#include "schemas.hpp"

namespace status_service {

namespace json = userver::formats::json;

// Parse V1CurrentUser from JSON
V1CurrentUser Parse(const json::Value& json,
                    userver::formats::parse::To<V1CurrentUser>);

// Parse V1UserStatus from JSON
V1UserStatus Parse(const json::Value& json,
                   userver::formats::parse::To<V1UserStatus>);

// Parse V1UserStatusUpdateRequest from JSON
V1UserStatusUpdateRequest Parse(
    const json::Value& json,
    userver::formats::parse::To<V1UserStatusUpdateRequest>);

// Parse V1UserStatusByLoginRequest from JSON
V1UserStatusByLoginRequest Parse(
    const json::Value& json,
    userver::formats::parse::To<V1UserStatusByLoginRequest>);

// Serialize V1UserStatusUpdateResponse to JSON
json::Value Serialize(const V1UserStatusUpdateResponse& response,
                      userver::formats::serialize::To<json::Value>);

// Serialize V1UserStatusByLoginResponse to JSON
json::Value Serialize(const V1UserStatusByLoginResponse& response,
                      userver::formats::serialize::To<json::Value>);

// Serialize V1ErrorResponse to JSON
json::Value Serialize(const V1ErrorResponse& error,
                      userver::formats::serialize::To<json::Value>);

// Serialize V1UserStatus to JSON (for internal use)
json::Value Serialize(const V1UserStatus& status,
                      userver::formats::serialize::To<json::Value>);

}  // namespace status_service