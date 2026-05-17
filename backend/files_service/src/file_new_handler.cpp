#include "file_new_handler.hpp"

#include <userver/components/component.hpp>
#include <userver/server/handlers/exceptions.hpp>
#include <userver/server/http/http_status.hpp>
#include "file_storage_component.hpp"
#include "json_utils.hpp"

namespace files_service {

FileNewHandler::FileNewHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      storage_(context.FindComponent<FileStorageComponent>()) {}

std::string FileNewHandler::HandleRequestThrow(
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
  auto file_request = request_body.As<V1FileNewRequest>();

  return HandleUploadFile(request, file_request);
}

std::string FileNewHandler::HandleUploadFile(
    const userver::server::http::HttpRequest& http_request,
    const V1FileNewRequest& request) const {
  // Validate required fields
  if (request.login.empty()) {
    http_request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kBadRequest);
    V1ErrorResponse error{"Field 'login' is required"};
    return userver::formats::json::ToString(Serialize(
        error,
        userver::formats::serialize::To<userver::formats::json::Value>{}));
  }
  if (request.filename.empty()) {
    http_request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kBadRequest);
    V1ErrorResponse error{"Field 'filename' is required"};
    return userver::formats::json::ToString(Serialize(
        error,
        userver::formats::serialize::To<userver::formats::json::Value>{}));
  }
  if (request.content.empty()) {
    http_request.GetHttpResponse().SetStatus(
        userver::server::http::HttpStatus::kBadRequest);
    V1ErrorResponse error{"Field 'content' is required"};
    return userver::formats::json::ToString(Serialize(
        error,
        userver::formats::serialize::To<userver::formats::json::Value>{}));
  }

  // Create V1File from request
  V1File file;
  file.login = request.login;
  file.filename = request.filename;
  file.content = request.content;
  file.mime_type = request.mime_type;
  file.size = request.size;

  // Store file and get URI
  std::string uri = storage_.StoreFile(file);

  // Create response
  V1FileNewResponse response;
  // Create current_user from login (token is empty since no authentication for
  // upload)
  response.current_user.token = "";  // Empty token (not authorized)
  response.current_user.login = request.login;
  response.current_user.name = "User";  // Default name
  response.uri = uri;
  response.file = file;

  // Set response content type
  http_request.GetHttpResponse().SetContentType(
      userver::http::content_type::kApplicationJson);

  // Serialize response
  auto json_response =
      userver::formats::json::ValueBuilder(response).ExtractValue();
  return userver::formats::json::ToString(json_response);
}

}  // namespace files_service