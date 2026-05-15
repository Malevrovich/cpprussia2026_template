#pragma once

#include <userver/server/handlers/http_handler_base.hpp>
#include "schemas.hpp"

namespace reactions_service {

class ReactionsStorageComponent;

class LikeTriggerHandler final
    : public userver::server::handlers::HttpHandlerBase {
 public:
  static constexpr std::string_view kName = "handler-like-trigger";

  LikeTriggerHandler(const userver::components::ComponentConfig& config,
                     const userver::components::ComponentContext& context);

  std::string HandleRequestThrow(
      const userver::server::http::HttpRequest&,
      userver::server::request::RequestContext&) const override;

 private:
  std::string HandleLikeTrigger(
      const userver::server::http::HttpRequest& http_request) const;

  ReactionsStorageComponent& storage_;
};

}  // namespace reactions_service