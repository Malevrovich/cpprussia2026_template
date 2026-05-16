#pragma once

#include <userver/server/handlers/http_handler_base.hpp>
#include "schemas.hpp"
#include "status_storage_component.hpp"

namespace status_service {

class StatusByLoginHandler final
    : public userver::server::handlers::HttpHandlerBase {
 public:
  static constexpr std::string_view kName = "handler-status-by-login";

  StatusByLoginHandler(const userver::components::ComponentConfig& config,
                       const userver::components::ComponentContext& context);

  std::string HandleRequestThrow(
      const userver::server::http::HttpRequest&,
      userver::server::request::RequestContext&) const override;

 private:
  // Validate token (basic validation - must exist and not be empty)
  void ValidateToken(const std::string& token) const;

  // Check if requester can view the status (owner can view private, anyone can
  // view public)
  bool CanViewStatus(const V1CurrentUser& requester,
                     const V1Login& target_login,
                     const UserStatusRecord& status_record) const;

  StatusStorageComponent& storage_;
};

}  // namespace status_service