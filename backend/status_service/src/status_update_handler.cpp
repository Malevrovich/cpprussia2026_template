#include "status_update_handler.hpp"
#include <userver/components/component.hpp>
#include <userver/server/handlers/exceptions.hpp>
#include <userver/utils/datetime.hpp>
#include "../../common/jwt_validation/jwt_validator.hpp"
#include "json_utils.hpp"
#include "status_storage_component.hpp"

namespace status_service {

namespace {

constexpr std::chrono::hours kDefaultTtl{24};

}  // namespace

StatusUpdateHandler::StatusUpdateHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      storage_(context.FindComponent<StatusStorageComponent>()) {}

std::string StatusUpdateHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& http_request,
    userver::server::request::RequestContext&) const {
  // Set response content type
  http_request.GetHttpResponse().SetContentType(
      userver::http::content_type::kApplicationJson);

  try {
    // Parse request
    auto request_body = http_request.RequestBody();
    auto json = userver::formats::json::FromString(request_body);
    auto request = json.As<V1UserStatusUpdateRequest>();

    // Validate token using common library
    common::jwt::JwtValidator::ValidateToken(request.current_user.token);

    // Calculate expiration time
    auto expires_at = CalculateExpiresAt(request);

    // Store status
    storage_.StoreStatus(request.current_user.login, request.status,
                         expires_at);

    // Prepare response
    V1UserStatusUpdateResponse response;
    response.success = true;
    response.updated_at = std::chrono::system_clock::now();
    response.expires_at = expires_at;

    return userver::formats::json::ToString(Serialize(
        response,
        userver::formats::serialize::To<userver::formats::json::Value>{}));

  } catch (const userver::formats::json::ParseException& e) {
    throw userver::server::handlers::ClientError(
        userver::server::handlers::ExternalBody{
            userver::formats::json::ToString(Serialize(
                V1ErrorResponse{"invalid_request", "Invalid JSON format"},
                userver::formats::serialize::To<
                    userver::formats::json::Value>{}))},
        userver::server::handlers::HandlerErrorCode::kClientError);
  } catch (const userver::formats::json::Exception& e) {
    // Catch other JSON-related errors (e.g., invalid enum values, type
    // mismatches)
    throw userver::server::handlers::ClientError(
        userver::server::handlers::ExternalBody{
            userver::formats::json::ToString(
                Serialize(V1ErrorResponse{"invalid_request", e.what()},
                          userver::formats::serialize::To<
                              userver::formats::json::Value>{}))},
        userver::server::handlers::HandlerErrorCode::kClientError);
  } catch (const userver::server::handlers::Unauthorized&) {
    throw;
  } catch (const userver::server::handlers::ClientError&) {
    throw;
  } catch (const userver::server::handlers::ResourceNotFound&) {
    throw;
  } catch (const std::runtime_error& e) {
    // Catch runtime errors from JSON parsing (e.g., invalid enum values)
    // Convert to ClientError (400) instead of InternalServerError (500)
    throw userver::server::handlers::ClientError(
        userver::server::handlers::ExternalBody{
            userver::formats::json::ToString(
                Serialize(V1ErrorResponse{"invalid_request", e.what()},
                          userver::formats::serialize::To<
                              userver::formats::json::Value>{}))},
        userver::server::handlers::HandlerErrorCode::kClientError);
  } catch (const std::exception& e) {
    throw userver::server::handlers::InternalServerError(
        userver::server::handlers::ExternalBody{
            userver::formats::json::ToString(
                Serialize(V1ErrorResponse{"internal_error", e.what()},
                          userver::formats::serialize::To<
                              userver::formats::json::Value>{}))});
  }
}

std::optional<std::chrono::system_clock::time_point>
StatusUpdateHandler::CalculateExpiresAt(
    const V1UserStatusUpdateRequest&) const {
  // For now, use default TTL of 24 hours
  // In the future, this could be configurable or come from the request
  return std::chrono::system_clock::now() + kDefaultTtl;
}

}  // namespace status_service