#include "json_utils.hpp"
#include <userver/formats/json.hpp>
#include <userver/formats/parse/common_containers.hpp>
#include <userver/formats/serialize/common_containers.hpp>

namespace auth_service {

namespace json = userver::formats::json;

V1UserRegistrationRequest Parse(
    const json::Value& json,
    userver::formats::parse::To<V1UserRegistrationRequest>) {
  V1UserRegistrationRequest request;
  request.login = json["login"].As<std::string>();
  request.name = json["name"].As<std::string>();
  request.email = json["email"].As<std::string>();
  request.phone = json["phone"].As<std::string>();
  request.password = json["password"].As<std::string>();

  // Validation
  if (request.login.size() < 3) {
    throw std::runtime_error("login must be at least 3 characters");
  }
  if (request.name.empty()) {
    throw std::runtime_error("name must not be empty");
  }
  if (request.email.size() < 3) {
    throw std::runtime_error("email must be at least 3 characters");
  }
  if (request.phone.size() < 3) {
    throw std::runtime_error("phone must be at least 3 characters");
  }
  if (request.password.size() < 6) {
    throw std::runtime_error("password must be at least 6 characters");
  }
  // TODO: email format validation

  return request;
}

V1UserAuthorizationRequest Parse(
    const json::Value& json,
    userver::formats::parse::To<V1UserAuthorizationRequest>) {
  V1UserAuthorizationRequest request;
  request.login = json["login"].As<std::string>();
  request.password = json["password"].As<std::string>();

  if (request.login.size() < 3) {
    throw std::runtime_error("login must be at least 3 characters");
  }
  if (request.password.size() < 6) {
    throw std::runtime_error("password must be at least 6 characters");
  }

  return request;
}

json::Value Serialize(const V1AuthorizedUser& user,
                      userver::formats::serialize::To<json::Value>) {
  json::ValueBuilder builder;
  builder["login"] = user.login;
  builder["name"] = user.name;
  builder["token"] = user.token;
  return builder.ExtractValue();
}

json::Value Serialize(const V1UserAuthorizationResponse& response,
                      userver::formats::serialize::To<json::Value>) {
  json::ValueBuilder builder;
  builder["current_user"] = response.current_user;
  return builder.ExtractValue();
}

json::Value Serialize(const V1Error& error,
                      userver::formats::serialize::To<json::Value>) {
  json::ValueBuilder builder;
  builder["code"] = error.code;
  builder["message"] = error.message;
  if (error.details.has_value()) {
    json::ValueBuilder detailsBuilder;
    for (const auto& [key, value] : *error.details) {
      detailsBuilder[key] = value;
    }
    builder["details"] = detailsBuilder.ExtractValue();
  }
  return builder.ExtractValue();
}

}  // namespace auth_service