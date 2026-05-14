#pragma once

#include <userver/formats/json.hpp>
#include <userver/formats/parse/common_containers.hpp>
#include <userver/formats/serialize/common_containers.hpp>
#include "schemas.hpp"

namespace auth_service {

namespace json = userver::formats::json;

// Parse V1UserRegistrationRequest from JSON
V1UserRegistrationRequest Parse(
    const json::Value& json,
    userver::formats::parse::To<V1UserRegistrationRequest>);

// Parse V1UserAuthorizationRequest from JSON
V1UserAuthorizationRequest Parse(
    const json::Value& json,
    userver::formats::parse::To<V1UserAuthorizationRequest>);

// Serialize V1AuthorizedUser to JSON
json::Value Serialize(const V1AuthorizedUser& user,
                      userver::formats::serialize::To<json::Value>);

// Serialize V1UserAuthorizationResponse to JSON
json::Value Serialize(const V1UserAuthorizationResponse& response,
                      userver::formats::serialize::To<json::Value>);

// Serialize V1Error to JSON
json::Value Serialize(const V1Error& error,
                      userver::formats::serialize::To<json::Value>);

}  // namespace auth_service