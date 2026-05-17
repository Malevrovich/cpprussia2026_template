#include "json_utils.hpp"

#include <userver/formats/json.hpp>

namespace files_service {

V1CurrentUser Parse(const json::Value& json,
                    userver::formats::parse::To<V1CurrentUser>) {
  V1CurrentUser result;
  result.token = json["token"].As<std::string>();
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

V1FileByUriResponse Parse(const json::Value& json,
                          userver::formats::parse::To<V1FileByUriResponse>) {
  V1FileByUriResponse result;
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

V1FileMetadata Parse(const json::Value& json,
                     userver::formats::parse::To<V1FileMetadata>) {
  V1FileMetadata result;
  result.uri = json["uri"].As<std::string>();
  result.login = json["login"].As<std::string>();
  result.filename = json["filename"].As<std::string>();
  if (json.HasMember("mime_type")) {
    result.mime_type = json["mime_type"].As<std::string>();
  }
  if (json.HasMember("size")) {
    result.size = json["size"].As<int64_t>();
  }
  return result;
}

V1FileListRequest Parse(const json::Value& json,
                        userver::formats::parse::To<V1FileListRequest>) {
  V1FileListRequest result;
  result.current_user = json["current_user"].As<V1CurrentUser>();
  if (json.HasMember("login")) {
    result.login = json["login"].As<V1Login>();
  }
  return result;
}

V1FileListResponse Parse(const json::Value& json,
                         userver::formats::parse::To<V1FileListResponse>) {
  V1FileListResponse result;
  for (const auto& item : json["files"]) {
    result.files.push_back(item.As<V1FileMetadata>());
  }
  return result;
}

json::Value Serialize(const V1CurrentUser& user,
                      userver::formats::serialize::To<json::Value>) {
  userver::formats::json::ValueBuilder builder;
  builder["token"] = user.token;
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
  builder["login"] = response.login;
  builder["filename"] = response.filename;
  builder["content"] = response.content;
  if (response.mime_type.has_value()) {
    builder["mime_type"] = response.mime_type.value();
  }
  if (response.size.has_value()) {
    builder["size"] = response.size.value();
  }
  return builder.ExtractValue();
}

json::Value Serialize(const V1FileMetadata& metadata,
                      userver::formats::serialize::To<json::Value>) {
  userver::formats::json::ValueBuilder builder;
  builder["uri"] = metadata.uri;
  builder["login"] = metadata.login;
  builder["filename"] = metadata.filename;
  if (metadata.mime_type.has_value()) {
    builder["mime_type"] = metadata.mime_type.value();
  }
  if (metadata.size.has_value()) {
    builder["size"] = metadata.size.value();
  }
  return builder.ExtractValue();
}

json::Value Serialize(const V1FileListResponse& response,
                      userver::formats::serialize::To<json::Value>) {
  userver::formats::json::ValueBuilder builder;
  builder["files"] = response.files;
  return builder.ExtractValue();
}

json::Value Serialize(const V1ErrorResponse& error,
                      userver::formats::serialize::To<json::Value>) {
  userver::formats::json::ValueBuilder builder;
  builder["error"] = error.error;
  return builder.ExtractValue();
}

}  // namespace files_service