#include "get_reactions_handler.hpp"

#include <userver/components/component.hpp>
#include <userver/http/common_headers.hpp>
#include <userver/server/handlers/exceptions.hpp>
#include <userver/server/http/http_request.hpp>
#include <userver/server/http/http_response.hpp>
#include <userver/tracing/span.hpp>

#include "json_utils.hpp"
#include "reactions_storage_component.hpp"

namespace reactions_service {

GetReactionsHandler::GetReactionsHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : userver::server::handlers::HttpHandlerBase(config, context),
      storage_(context.FindComponent<ReactionsStorageComponent>()) {
  LOG_INFO() << "GetReactionsHandler initialized";
}

std::string GetReactionsHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
  // Parse path parameters
  V1ChannelId channel_id = 0;
  V1MessageId message_id = 0;

  try {
    channel_id = std::stoll(request.GetPathArg("channel_id"));
    message_id = std::stoll(request.GetPathArg("message_id"));
  } catch (const std::exception& e) {
    throw userver::server::handlers::ClientError(
        userver::server::handlers::ExternalBody{
            "Invalid channel_id or message_id: must be integers"});
  }

  return HandleGetReactions(request, channel_id, message_id);
}

std::string GetReactionsHandler::HandleGetReactions(
    const userver::server::http::HttpRequest& http_request,
    V1ChannelId channel_id, V1MessageId message_id) const {
  auto& response = http_request.GetHttpResponse();
  response.SetContentType(userver::http::content_type::kApplicationJson);

  try {
    // Validate HTTP method
    if (http_request.GetMethod() != userver::server::http::HttpMethod::kGet) {
      throw userver::server::handlers::ClientError(
          userver::server::handlers::ExternalBody{
              "Method not allowed. Use GET."});
    }

    // Check if message exists (according to spec, all messages exist)
    if (!storage_.MessageExists(channel_id, message_id)) {
      throw userver::server::handlers::ResourceNotFound(
          userver::server::handlers::ExternalBody{"Message not found"});
    }

    // Get reactions from storage
    auto reactions = storage_.GetReactions(channel_id, message_id);

    // Build response
    V1GetReactionsResponse response_body;
    response_body.reactions = std::move(reactions);

    // Return success response
    return userver::formats::json::ToString(Serialize(
        response_body,
        userver::formats::serialize::To<userver::formats::json::Value>{}));

  } catch (const userver::server::handlers::CustomHandlerException& e) {
    // Re-throw custom exceptions
    throw;
  } catch (const std::exception& e) {
    // Handle any other exceptions
    LOG_ERROR() << "Unexpected error in GetReactionsHandler: " << e.what();
    throw userver::server::handlers::InternalServerError(
        userver::server::handlers::ExternalBody{"Internal server error: " +
                                                std::string(e.what())});
  }
}

}  // namespace reactions_service