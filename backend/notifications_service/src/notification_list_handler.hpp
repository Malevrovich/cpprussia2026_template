#pragma once

#include <userver/server/handlers/http_handler_base.hpp>
#include "schemas.hpp"

namespace notifications_service {

class NotificationStorageComponent;

class NotificationListHandler final
    : public userver::server::handlers::HttpHandlerBase {
 public:
  static constexpr std::string_view kName = "handler-notification-list";

  NotificationListHandler(const userver::components::ComponentConfig& config,
                          const userver::components::ComponentContext& context);

  std::string HandleRequestThrow(
      const userver::server::http::HttpRequest&,
      userver::server::request::RequestContext&) const override;

 private:
  std::string HandleListNotifications(
      const userver::server::http::HttpRequest& http_request,
      const V1ChannelNotificationListRequest& request) const;

  NotificationStorageComponent& storage_;
};

}  // namespace notifications_service