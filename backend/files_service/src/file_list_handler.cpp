#include "file_list_handler.hpp"

#include <userver/components/component.hpp>
#include <userver/server/handlers/exceptions.hpp>
#include <userver/server/http/http_status.hpp>
#include "../../common/jwt_validation/jwt_validator.hpp"
#include "file_storage_component.hpp"
#include "json_utils.hpp"

namespace files_service {

FileListHandler::FileListHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      storage_(context.FindComponent<FileStorageComponent>()) {}

std::string FileListHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
  // Only POST method is allowed
  if (request.GetMethod() != userver::server::http::HttpMethod::kPost) {
    request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kMethodNotAllowed);
    V1ErrorResponse error{"Method not allowed. Use POST."};
    return userver::formats::json::ToString(Serialize(
        error,
        userver::formats::serialize::To<userver::formats::json::Value>{}));
  }

  // Parse request body
  auto request_body = userver::formats::json::FromString(request.RequestBody());
  auto list_request = request_body.As<V1FileListRequest>();

  return HandleListFiles(request, list_request);
}

std::string FileListHandler::HandleListFiles(
    const userver::server::http::HttpRequest& http_request,
    const V1FileListRequest& request) const {
  // Validate token using common library
  common::jwt::JwtValidator::ValidateToken(request.current_user.token);

  // List files with optional filter
  auto files = storage_.ListFiles(request.login);

  // Create response
  V1FileListResponse response;
  response.files = std::move(files);

  // Set response content type
  http_request.GetHttpResponse().SetContentType(
      userver::http::content_type::kApplicationJson);

  // Serialize response
  auto json_response =
      userver::formats::json::ValueBuilder(response).ExtractValue();
  return userver::formats::json::ToString(json_response);
}

}  // namespace files_service