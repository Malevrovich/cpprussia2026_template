#pragma once

#include <userver/server/handlers/http_handler_base.hpp>
#include "schemas.hpp"

namespace notifications_service {

class NotificationStorageComponent;

class NotificationReadHandler final
    : public userver::server::handlers::HttpHandlerBase {
 public:
  static constexpr std::string_view kName = "handler-notification-read";

  NotificationReadHandler(const userver::components::ComponentConfig& config,
                          const userver::components::ComponentContext& context);

  std::string HandleRequestThrow(
      const userver::server::http::HttpRequest&,
      userver::server::request::RequestContext&) const override;

 private:
  std::string HandleMarkAsRead(
      const userver::server::http::HttpRequest& http_request,
      const V1ChannelNotificationReadRequest& request) const;

  NotificationStorageComponent& storage_;
};

}  // namespace notifications_service