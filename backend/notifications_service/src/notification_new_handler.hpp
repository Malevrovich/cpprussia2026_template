#pragma once

#include <userver/server/handlers/http_handler_base.hpp>
#include "schemas.hpp"

namespace notifications_service {

class NotificationStorageComponent;

class NotificationNewHandler final
    : public userver::server::handlers::HttpHandlerBase {
 public:
  static constexpr std::string_view kName = "handler-notification-new";

  NotificationNewHandler(const userver::components::ComponentConfig& config,
                         const userver::components::ComponentContext& context);

  std::string HandleRequestThrow(
      const userver::server::http::HttpRequest&,
      userver::server::request::RequestContext&) const override;

 private:
  std::string HandleCreateNotification(
      const userver::server::http::HttpRequest& http_request,
      const V1ChannelNotificationNewRequest& request) const;

  NotificationStorageComponent& storage_;
};

}  // namespace notifications_service