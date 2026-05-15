#include "message_by_timestamp_handler.hpp"
#include <userver/components/component_context.hpp>
#include <userver/formats/json.hpp>
#include <userver/logging/log.hpp>
#include <userver/server/http/http_status.hpp>
#include <userver/utils/datetime.hpp>
#include "json_utils.hpp"
#include "message_storage_component.hpp"

namespace messaging_service {

MessageByTimestampHandler::MessageByTimestampHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      storage_(context.FindComponent<MessageStorageComponent>()) {}

std::string MessageByTimestampHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
  request.GetHttpResponse().SetContentType(
      userver::http::content_type::kApplicationJson);

  try {
    // Parse request body
    auto json_body = userver::formats::json::FromString(request.RequestBody());
    auto get_messages_request =
        json_body.As<V1ChannelMessageByTimestampRequest>();

    // Call business logic
    return HandleGetMessages(request, get_messages_request);
  } catch (const std::exception& e) {
    LOG_ERROR() << "Message by timestamp error: " << e.what();
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

std::string MessageByTimestampHandler::HandleGetMessages(
    const userver::server::http::HttpRequest& http_request,
    const V1ChannelMessageByTimestampRequest& request) const {
  // Validate request
  if (request.limit < 1 || request.limit > 1000) {
    LOG_WARNING() << "Invalid limit: " << request.limit;
    http_request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kBadRequest);
    V1Error error;
    error.error = "Limit must be between 1 and 1000";
    error.code = 400;
    return userver::formats::json::ToString(Serialize(
        error,
        userver::formats::serialize::To<userver::formats::json::Value>{}));
  }

  if (request.from.empty()) {
    LOG_WARNING() << "Empty from timestamp";
    http_request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kBadRequest);
    V1Error error;
    error.error = "From timestamp is required";
    error.code = 400;
    return userver::formats::json::ToString(Serialize(
        error,
        userver::formats::serialize::To<userver::formats::json::Value>{}));
  }

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

  // Get messages from storage
  auto messages = storage_.GetMessagesByTimestamp(
      request.channel_id, request.from, request.to, request.limit);

  LOG_INFO() << "Retrieved " << messages.size() << " messages from channel "
             << request.channel_id;

  // Prepare response
  V1ChannelMessageByTimestampResponse response;
  response.messages = std::move(messages);

  // Simple pagination: if we got exactly limit messages, there might be more
  response.has_more =
      response.messages.size() == static_cast<size_t>(request.limit);

  // For now, we don't implement complex cursor pagination
  // In a real implementation, we would generate a cursor based on the last
  // message
  response.next_cursor = std::nullopt;

  http_request.GetHttpResponse().SetStatus(
      userver::server::http::HttpStatus::kOk);
  return userver::formats::json::ToString(Serialize(
      response,
      userver::formats::serialize::To<userver::formats::json::Value>{}));
}

}  // namespace messaging_service