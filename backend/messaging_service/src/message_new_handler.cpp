#include "message_new_handler.hpp"
#include <userver/components/component_context.hpp>
#include <userver/formats/json.hpp>
#include <userver/logging/log.hpp>
#include <userver/server/http/http_status.hpp>
#include <userver/utils/datetime.hpp>
#include "../../common/jwt_validation/jwt_validator.hpp"
#include "json_utils.hpp"
#include "message_storage_component.hpp"

namespace messaging_service {

MessageNewHandler::MessageNewHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      storage_(context.FindComponent<MessageStorageComponent>()) {}

std::string MessageNewHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
  request.GetHttpResponse().SetContentType(
      userver::http::content_type::kApplicationJson);

  try {
    // Parse request body
    auto json_body = userver::formats::json::FromString(request.RequestBody());
    auto new_message_request = json_body.As<V1ChannelMessageNewRequest>();

    // Call business logic
    return HandleNewMessage(request, new_message_request);
  } catch (const std::exception& e) {
    LOG_ERROR() << "Message new error: " << e.what();
    request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kBadRequest);
    V1Error error;
    error.error = e.what();
    error.code = 400;
    return userver::formats::json::ToString(Serialize(
        error,
        userver::formats::serialize::To<userver::formats::json::Value>{}));
  }
}

std::string MessageNewHandler::HandleNewMessage(
    const userver::server::http::HttpRequest& http_request,
    const V1ChannelMessageNewRequest& request) const {
  // Validate request
  if (request.message.empty()) {
    LOG_WARNING() << "Empty message received";
    http_request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kBadRequest);
    V1Error error;
    error.error = "Message cannot be empty";
    error.code = 400;
    return userver::formats::json::ToString(Serialize(
        error,
        userver::formats::serialize::To<userver::formats::json::Value>{}));
  }

  if (request.current_user.login.empty()) {
    LOG_WARNING() << "Empty user login received";
    http_request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kBadRequest);
    V1Error error;
    error.error = "User login cannot be empty";
    error.code = 400;
    return userver::formats::json::ToString(Serialize(
        error,
        userver::formats::serialize::To<userver::formats::json::Value>{}));
  }

  // Validate token using common library
  common::jwt::JwtValidator::ValidateToken(request.current_user.token);

  // Check if channel exists (all channels exist per spec)
  if (!storage_.ChannelExists(request.channel_id)) {
    LOG_WARNING() << "Channel not found: " << request.channel_id;
    http_request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kNotFound);
    V1Error error;
    error.error = "Channel not found";
    error.code = 404;
    return userver::formats::json::ToString(Serialize(
        error,
        userver::formats::serialize::To<userver::formats::json::Value>{}));
  }

  // Create message object
  V1ChannelMessage message;
  message.current_user = request.current_user;
  message.id = storage_.GetNextMessageId();
  message.timestamp =
      userver::utils::datetime::Timestring(userver::utils::datetime::Now());
  message.message = request.message;

  // Store the message
  storage_.StoreMessage(request.channel_id, message);

  LOG_INFO() << "New message stored in channel " << request.channel_id
             << " with ID " << message.id << " from user "
             << request.current_user.login;

  // Prepare response
  V1ChannelMessageNewResponse response;
  response.message_id = message.id;

  http_request.GetHttpResponse().SetStatus(
      userver::server::http::HttpStatus::kOk);
  return userver::formats::json::ToString(Serialize(
      response,
      userver::formats::serialize::To<userver::formats::json::Value>{}));
}

}  // namespace messaging_service