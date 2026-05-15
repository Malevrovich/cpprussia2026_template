#pragma once

#include <userver/server/handlers/http_handler_base.hpp>
#include "schemas.hpp"

namespace auth_service {

class UserStorageComponent;

class RegistrationHandler final
    : public userver::server::handlers::HttpHandlerBase {
 public:
  static constexpr std::string_view kName = "handler-registration";

  RegistrationHandler(const userver::components::ComponentConfig& config,
                      const userver::components::ComponentContext& context);

  std::string HandleRequestThrow(
      const userver::server::http::HttpRequest&,
      userver::server::request::RequestContext&) const override;

 private:
  std::string HandleRegistration(
      const userver::server::http::HttpRequest& http_request,
      const V1UserRegistrationRequest& request) const;

  UserStorageComponent& storage_;
};

}  // namespace auth_service