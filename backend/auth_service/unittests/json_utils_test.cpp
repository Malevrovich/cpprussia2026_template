#include "json_utils.hpp"
#include "schemas.hpp"

#include <userver/formats/json.hpp>
#include <userver/utest/utest.hpp>

namespace auth_service {

namespace json = userver::formats::json;

UTEST(JsonUtils, ParseRegistrationRequestValid) {
  const std::string valid_json = R"({
        "login": "testuser",
        "name": "Test User",
        "email": "test@example.com",
        "phone": "+1234567890",
        "password": "securePassword123"
    })";
  auto json_value = json::FromString(valid_json);
  auto request = json_value.As<V1UserRegistrationRequest>();
  EXPECT_EQ(request.login, "testuser");
  EXPECT_EQ(request.name, "Test User");
  EXPECT_EQ(request.email, "test@example.com");
  EXPECT_EQ(request.phone, "+1234567890");
  EXPECT_EQ(request.password, "securePassword123");
}

UTEST(JsonUtils, ParseRegistrationRequestShortLogin) {
  const std::string json = R"({
        "login": "ab",
        "name": "Test",
        "email": "test@example.com",
        "phone": "+1234567890",
        "password": "securePassword123"
    })";
  auto json_value = json::FromString(json);
  EXPECT_THROW(json_value.As<V1UserRegistrationRequest>(), std::runtime_error);
}

UTEST(JsonUtils, ParseRegistrationRequestShortPassword) {
  const std::string json = R"({
        "login": "testuser",
        "name": "Test",
        "email": "test@example.com",
        "phone": "+1234567890",
        "password": "short"
    })";
  auto json_value = json::FromString(json);
  EXPECT_THROW(json_value.As<V1UserRegistrationRequest>(), std::runtime_error);
}

UTEST(JsonUtils, ParseRegistrationRequestMissingField) {
  const std::string json = R"({
        "login": "testuser",
        "name": "Test",
        "email": "test@example.com",
        "phone": "+1234567890"
        // missing password
    })";
  auto json_value = json::FromString(json);
  // Will throw because key not found (userver's parsing throws)
  EXPECT_THROW(json_value.As<V1UserRegistrationRequest>(), std::exception);
}

UTEST(JsonUtils, ParseAuthorizationRequestValid) {
  const std::string json = R"({
        "login": "testuser",
        "password": "securePassword123"
    })";
  auto json_value = json::FromString(json);
  auto request = json_value.As<V1UserAuthorizationRequest>();
  EXPECT_EQ(request.login, "testuser");
  EXPECT_EQ(request.password, "securePassword123");
}

UTEST(JsonUtils, ParseAuthorizationRequestShortLogin) {
  const std::string json = R"({
        "login": "ab",
        "password": "securePassword123"
    })";
  auto json_value = json::FromString(json);
  EXPECT_THROW(json_value.As<V1UserAuthorizationRequest>(), std::runtime_error);
}

UTEST(JsonUtils, ParseAuthorizationRequestShortPassword) {
  const std::string json = R"({
        "login": "testuser",
        "password": "short"
    })";
  auto json_value = json::FromString(json);
  EXPECT_THROW(json_value.As<V1UserAuthorizationRequest>(), std::runtime_error);
}

UTEST(JsonUtils, SerializeAuthorizedUser) {
  V1AuthorizedUser user{.login = "testuser",
                        .name = "Test User",
                        .token =
                            "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9."
                            "eyJzdWIiOiJ0ZXN0dXNlciIsImlhdCI6MTYzOTM..."};
  auto json = Serialize(user, userver::formats::serialize::To<json::Value>{});
  EXPECT_EQ(json["login"].As<std::string>(), "testuser");
  EXPECT_EQ(json["name"].As<std::string>(), "Test User");
  EXPECT_EQ(json["token"].As<std::string>(), user.token);
}

UTEST(JsonUtils, SerializeError) {
  V1Error error{.code = "validation_error",
                .message = "Invalid input",
                .details = std::nullopt};
  auto json = Serialize(error, userver::formats::serialize::To<json::Value>{});
  EXPECT_EQ(json["code"].As<std::string>(), "validation_error");
  EXPECT_EQ(json["message"].As<std::string>(), "Invalid input");
  EXPECT_FALSE(json.HasMember("details"));
}

UTEST(JsonUtils, SerializeErrorWithDetails) {
  std::unordered_map<std::string, std::string> details{{"field", "login"},
                                                       {"reason", "too short"}};
  V1Error error{.code = "validation_error",
                .message = "Invalid input",
                .details = details};
  auto json = Serialize(error, userver::formats::serialize::To<json::Value>{});
  EXPECT_EQ(json["code"].As<std::string>(), "validation_error");
  EXPECT_EQ(json["message"].As<std::string>(), "Invalid input");
  EXPECT_TRUE(json.HasMember("details"));
  auto details_json = json["details"];
  EXPECT_EQ(details_json["field"].As<std::string>(), "login");
  EXPECT_EQ(details_json["reason"].As<std::string>(), "too short");
}

}  // namespace auth_service