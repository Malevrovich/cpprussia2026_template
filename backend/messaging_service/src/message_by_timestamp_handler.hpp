#pragma once

#include <userver/server/handlers/http_handler_base.hpp>
#include "schemas.hpp"

namespace messaging_service {

class MessageStorageComponent;

class MessageByTimestampHandler final
    : public userver::server::handlers::HttpHandlerBase {
 public:
  static constexpr std::string_view kName = "handler-message-by-timestamp";

  MessageByTimestampHandler(
      const userver::components::ComponentConfig& config,
      const userver::components::ComponentContext& context);

  std::string HandleRequestThrow(
      const userver::server::http::HttpRequest&,
      userver::server::request::RequestContext&) const override;

 private:
  std::string HandleGetMessages(
      const userver::server::http::HttpRequest& http_request,
      const V1ChannelMessageByTimestampRequest& request) const;

  MessageStorageComponent& storage_;
};

}  // namespace messaging_service