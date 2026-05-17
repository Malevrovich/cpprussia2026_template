#include "like_trigger_handler.hpp"

#include <userver/components/component.hpp>
#include <userver/http/common_headers.hpp>
#include <userver/server/handlers/exceptions.hpp>
#include <userver/server/http/http_request.hpp>
#include <userver/server/http/http_response.hpp>
#include <userver/tracing/span.hpp>

#include "../../common/jwt_validation/jwt_validator.hpp"
#include "json_utils.hpp"
#include "reactions_storage_component.hpp"

namespace reactions_service {

LikeTriggerHandler::LikeTriggerHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : userver::server::handlers::HttpHandlerBase(config, context),
      storage_(context.FindComponent<ReactionsStorageComponent>()) {
  LOG_INFO() << "LikeTriggerHandler initialized";
}

std::string LikeTriggerHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
  return HandleLikeTrigger(request);
}

std::string LikeTriggerHandler::HandleLikeTrigger(
    const userver::server::http::HttpRequest& http_request) const {
  auto& response = http_request.GetHttpResponse();
  response.SetContentType(userver::http::content_type::kApplicationJson);

  try {
    // Validate HTTP method
    if (http_request.GetMethod() != userver::server::http::HttpMethod::kPost) {
      throw userver::server::handlers::ClientError(
          userver::server::handlers::ExternalBody{
              "Method not allowed. Use POST."});
    }

    // Parse request body
    auto request_body =
        Parse(userver::formats::json::FromString(http_request.RequestBody()),
              userver::formats::parse::To<V1LikeTriggerRequest>{});
    // Validate request parameters
    if (request_body.current_user.login.size() < 3) {
      throw userver::server::handlers::ClientError(
          userver::server::handlers::ExternalBody{
              "Invalid login: min 3 characters"});
    }
    if (request_body.idempotency_token.size() < 16 ||
        request_body.idempotency_token.size() > 256) {
      throw userver::server::handlers::ClientError(
          userver::server::handlers::ExternalBody{
              "Invalid idempotency_token: must be 16-256 characters"});
    }
    // Validate token using common library
    common::jwt::JwtValidator::ValidateToken(request_body.current_user.token);

    // Check if message exists (according to spec, all messages exist)
    if (!storage_.MessageExists(request_body.channel_id,
                                request_body.message_id)) {
      throw userver::server::handlers::ResourceNotFound(
          userver::server::handlers::ExternalBody{"Message not found"});
    }

    // Perform toggle operation
    auto [action, current_reaction] = storage_.ToggleReaction(request_body);

    // Build response
    V1LikeTriggerResponse response_body;
    response_body.action = action;
    response_body.current_user_reaction = current_reaction;

    // Return success response
    return userver::formats::json::ToString(Serialize(
        response_body,
        userver::formats::serialize::To<userver::formats::json::Value>{}));

  } catch (const userver::server::handlers::CustomHandlerException& e) {
    // Re-throw custom exceptions
    throw;
  } catch (const std::runtime_error& e) {
    // Handle idempotency token conflict
    if (std::string(e.what()).find("Idempotency token conflict") !=
        std::string::npos) {
      throw userver::server::handlers::ConflictError(
          userver::server::handlers::ExternalBody{e.what()});
    }
    // Handle invalid animation errors
    if (std::string(e.what()).find("Invalid animation") != std::string::npos) {
      throw userver::server::handlers::ClientError(
          userver::server::handlers::ExternalBody{e.what()});
    }
    // Handle other runtime errors
    throw userver::server::handlers::InternalServerError(
        userver::server::handlers::ExternalBody{e.what()});
  } catch (const userver::formats::json::ParseException& e) {
    // Handle JSON parsing errors
    throw userver::server::handlers::ClientError(
        userver::server::handlers::ExternalBody{"Invalid JSON: " +
                                                std::string(e.what())});
  } catch (const std::exception& e) {
    // Handle any other exceptions
    LOG_ERROR() << "Unexpected error in LikeTriggerHandler: " << e.what();
    throw userver::server::handlers::InternalServerError(
        userver::server::handlers::ExternalBody{"Internal server error: " +
                                                std::string(e.what())});
  }
}

}  // namespace reactions_service