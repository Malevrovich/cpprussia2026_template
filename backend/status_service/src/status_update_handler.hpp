#pragma once

#include <userver/server/handlers/http_handler_base.hpp>
#include "schemas.hpp"

namespace status_service {

class StatusStorageComponent;

class StatusUpdateHandler final
    : public userver::server::handlers::HttpHandlerBase {
 public:
  static constexpr std::string_view kName = "handler-status-update";

  StatusUpdateHandler(const userver::components::ComponentConfig& config,
                      const userver::components::ComponentContext& context);

  std::string HandleRequestThrow(
      const userver::server::http::HttpRequest&,
      userver::server::request::RequestContext&) const override;

 private:
  // Validate token (basic validation - must exist and not be empty)
  void ValidateToken(const std::string& token) const;

  // Calculate expiration time (default: 24 hours from now)
  std::optional<std::chrono::system_clock::time_point> CalculateExpiresAt(
      const V1UserStatusUpdateRequest& request) const;

  StatusStorageComponent& storage_;
};

}  // namespace status_service