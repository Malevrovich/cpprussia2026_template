#pragma once

#include <userver/formats/json.hpp>
#include <userver/formats/parse/common_containers.hpp>
#include <userver/formats/serialize/common_containers.hpp>
#include "schemas.hpp"

namespace files_service {

namespace json = userver::formats::json;

// Parse V1CurrentUser from JSON
V1CurrentUser Parse(const json::Value& json,
                    userver::formats::parse::To<V1CurrentUser>);

// Parse V1File from JSON
V1File Parse(const json::Value& json, userver::formats::parse::To<V1File>);

// Parse V1FileNewRequest from JSON
V1FileNewRequest Parse(const json::Value& json,
                       userver::formats::parse::To<V1FileNewRequest>);

// Parse V1FileByUriRequest from JSON
V1FileByUriRequest Parse(const json::Value& json,
                         userver::formats::parse::To<V1FileByUriRequest>);

// Parse V1FileByUriResponse from JSON (flat structure)
V1FileByUriResponse Parse(const json::Value& json,
                          userver::formats::parse::To<V1FileByUriResponse>);

// Parse V1FileMetadata from JSON
V1FileMetadata Parse(const json::Value& json,
                     userver::formats::parse::To<V1FileMetadata>);

// Parse V1FileListRequest from JSON
V1FileListRequest Parse(const json::Value& json,
                        userver::formats::parse::To<V1FileListRequest>);

// Parse V1FileListResponse from JSON
V1FileListResponse Parse(const json::Value& json,
                         userver::formats::parse::To<V1FileListResponse>);

// Serialize V1CurrentUser to JSON
json::Value Serialize(const V1CurrentUser& user,
                      userver::formats::serialize::To<json::Value>);

// Serialize V1File to JSON
json::Value Serialize(const V1File& file,
                      userver::formats::serialize::To<json::Value>);

// Serialize V1FileNewResponse to JSON
json::Value Serialize(const V1FileNewResponse& response,
                      userver::formats::serialize::To<json::Value>);

// Serialize V1FileByUriResponse to JSON (flat structure)
json::Value Serialize(const V1FileByUriResponse& response,
                      userver::formats::serialize::To<json::Value>);

// Serialize V1FileMetadata to JSON
json::Value Serialize(const V1FileMetadata& metadata,
                      userver::formats::serialize::To<json::Value>);

// Serialize V1FileListResponse to JSON
json::Value Serialize(const V1FileListResponse& response,
                      userver::formats::serialize::To<json::Value>);

// Serialize V1ErrorResponse to JSON
json::Value Serialize(const V1ErrorResponse& error,
                      userver::formats::serialize::To<json::Value>);

}  // namespace files_service