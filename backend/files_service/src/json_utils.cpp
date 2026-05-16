#include "json_utils.hpp"

#include <userver/formats/json.hpp>

namespace files_service {

V1CurrentUser Parse(const json::Value& json,
                    userver::formats::parse::To<V1CurrentUser>) {
  V1CurrentUser result;
  if (json.HasMember("token")) {
    result.token = json["token"].As<std::string>();
  }
  result.login = json["login"].As<std::string>();
  result.name = json["name"].As<std::string>();
  return result;
}

V1File Parse(const json::Value& json, userver::formats::parse::To<V1File>) {
  V1File result;
  result.login = json["login"].As<std::string>();
  result.filename = json["filename"].As<std::string>();
  result.content = json["content"].As<std::string>();
  if (json.HasMember("mime_type")) {
    result.mime_type = json["mime_type"].As<std::string>();
  }
  if (json.HasMember("size")) {
    result.size = json["size"].As<int64_t>();
  }
  return result;
}

V1FileNewRequest Parse(const json::Value& json,
                       userver::formats::parse::To<V1FileNewRequest>) {
  V1FileNewRequest result;
  result.login = json["login"].As<std::string>();
  result.filename = json["filename"].As<std::string>();
  result.content = json["content"].As<std::string>();
  if (json.HasMember("mime_type")) {
    result.mime_type = json["mime_type"].As<std::string>();
  }
  if (json.HasMember("size")) {
    result.size = json["size"].As<int64_t>();
  }
  return result;
}

V1FileByUriRequest Parse(const json::Value& json,
                         userver::formats::parse::To<V1FileByUriRequest>) {
  V1FileByUriRequest result;
  result.current_user = json["current_user"].As<V1CurrentUser>();
  result.uri = json["uri"].As<std::string>();
  return result;
}

json::Value Serialize(const V1CurrentUser& user,
                      userver::formats::serialize::To<json::Value>) {
  userver::formats::json::ValueBuilder builder;
  if (user.token.has_value()) {
    builder["token"] = user.token.value();
  }
  builder["login"] = user.login;
  builder["name"] = user.name;
  return builder.ExtractValue();
}

json::Value Serialize(const V1File& file,
                      userver::formats::serialize::To<json::Value>) {
  userver::formats::json::ValueBuilder builder;
  builder["login"] = file.login;
  builder["filename"] = file.filename;
  builder["content"] = file.content;
  if (file.mime_type.has_value()) {
    builder["mime_type"] = file.mime_type.value();
  }
  if (file.size.has_value()) {
    builder["size"] = file.size.value();
  }
  return builder.ExtractValue();
}

json::Value Serialize(const V1FileNewResponse& response,
                      userver::formats::serialize::To<json::Value>) {
  userver::formats::json::ValueBuilder builder;
  builder["current_user"] = response.current_user;
  builder["uri"] = response.uri;
  builder["file"] = response.file;
  return builder.ExtractValue();
}

json::Value Serialize(const V1FileByUriResponse& response,
                      userver::formats::serialize::To<json::Value>) {
  userver::formats::json::ValueBuilder builder;
  builder["file"] = response.file;
  return builder.ExtractValue();
}

json::Value Serialize(const V1ErrorResponse& error,
                      userver::formats::serialize::To<json::Value>) {
  userver::formats::json::ValueBuilder builder;
  builder["error"] = error.error;
  return builder.ExtractValue();
}

}  // namespace files_service