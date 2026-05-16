#pragma once

#include <userver/server/handlers/http_handler_base.hpp>
#include "schemas.hpp"

namespace files_service {

class FileStorageComponent;

class FileByUriHandler final
    : public userver::server::handlers::HttpHandlerBase {
 public:
  static constexpr std::string_view kName = "handler-file-by-uri";

  FileByUriHandler(const userver::components::ComponentConfig& config,
                   const userver::components::ComponentContext& context);

  std::string HandleRequestThrow(
      const userver::server::http::HttpRequest&,
      userver::server::request::RequestContext&) const override;

 private:
  std::string HandleGetFile(
      const userver::server::http::HttpRequest& http_request,
      const V1FileByUriRequest& request) const;

  FileStorageComponent& storage_;
};

}  // namespace files_service