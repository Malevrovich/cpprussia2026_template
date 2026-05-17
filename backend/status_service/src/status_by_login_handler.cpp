#include "status_by_login_handler.hpp"
#include <userver/components/component.hpp>
#include <userver/server/handlers/exceptions.hpp>
#include <userver/utils/datetime.hpp>
#include "../../common/jwt_validation/jwt_validator.hpp"
#include "json_utils.hpp"
#include "status_storage_component.hpp"

namespace status_service {

StatusByLoginHandler::StatusByLoginHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      storage_(context.FindComponent<StatusStorageComponent>()) {}

std::string StatusByLoginHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& http_request,
    userver::server::request::RequestContext&) const {
  // Set response content type
  http_request.GetHttpResponse().SetContentType(
      userver::http::content_type::kApplicationJson);

  try {
    // Parse request
    auto request_body = http_request.RequestBody();
    auto json = userver::formats::json::FromString(request_body);
    auto request = json.As<V1UserStatusByLoginRequest>();

    // Validate token using common library
    common::jwt::JwtValidator::ValidateToken(request.current_user.token);

    // Check if target user exists
    if (!storage_.UserExists(request.login)) {
      throw userver::server::handlers::ResourceNotFound(
          userver::server::handlers::ExternalBody{
              userver::formats::json::ToString(Serialize(
                  V1ErrorResponse{"not_found",
                                  "User with specified login not found"},
                  userver::formats::serialize::To<
                      userver::formats::json::Value>{}))});
    }

    // Get target user's status
    auto status_record = storage_.GetStatus(request.login);
    if (!status_record.has_value()) {
      // This shouldn't happen if UserExists returned true, but handle it anyway
      throw userver::server::handlers::ResourceNotFound(
          userver::server::handlers::ExternalBody{
              userver::formats::json::ToString(Serialize(
                  V1ErrorResponse{"not_found", "User status not found"},
                  userver::formats::serialize::To<
                      userver::formats::json::Value>{}))});
    }

    // Check if requester can view the status
    if (!CanViewStatus(request.current_user, request.login, *status_record)) {
      throw userver::server::handlers::ClientError(
          userver::server::handlers::ExternalBody{
              userver::formats::json::ToString(Serialize(
                  V1ErrorResponse{
                      "forbidden",
                      "You do not have permission to view this status"},
                  userver::formats::serialize::To<
                      userver::formats::json::Value>{}))},
          userver::server::handlers::HandlerErrorCode::kForbidden);
    }

    // Prepare response
    V1UserStatusByLoginResponse response;
    response.status = status_record->status;
    response.updated_at = status_record->updated_at;

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

bool StatusByLoginHandler::CanViewStatus(
    const V1CurrentUser& requester, const V1Login& target_login,
    const UserStatusRecord& status_record) const {
  // Owner can always view their own status
  if (requester.login == target_login) {
    return true;
  }

  // For non-owners, only public statuses are visible
  return status_record.status.visibility == V1Visibility::kPublic;
}

}  // namespace status_service