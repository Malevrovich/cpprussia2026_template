#pragma once

#include <userver/server/handlers/http_handler_base.hpp>
#include "schemas.hpp"

namespace reactions_service {

class ReactionsStorageComponent;

class GetReactionsHandler final
    : public userver::server::handlers::HttpHandlerBase {
 public:
  static constexpr std::string_view kName = "handler-get-reactions";

  GetReactionsHandler(const userver::components::ComponentConfig& config,
                      const userver::components::ComponentContext& context);

  std::string HandleRequestThrow(
      const userver::server::http::HttpRequest&,
      userver::server::request::RequestContext&) const override;

 private:
  std::string HandleGetReactions(
      const userver::server::http::HttpRequest& http_request,
      V1ChannelId channel_id, V1MessageId message_id) const;

  ReactionsStorageComponent& storage_;
};

}  // namespace reactions_service