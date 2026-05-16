---
description: >
  Step-by-step guide to write a typed HTTP client component that wraps
  userver clients::http::Client and exposes one C++ method per OpenAPI
  operation (CreateX, GetX, DeleteX, ...). This is the mirror image of
  create-http-handler: instead of receiving requests, the component issues
  them. Includes: component scaffolding, ctor obtaining
  components::HttpClient::GetHttpClient(), typed Request/Response/Error
  structs, JSON parse/serialize, per-operation method skeleton with
  CreateRequest().get()/post()/put()/delete_method(), timeouts/retries,
  status-code-to-exception mapping, static_config entries, and testsuite
  mocking.
  Trigger: http client wrapper, typed http client, openapi client by hand,
  outgoing http calls, call external service, clients::http::Client,
  components::HttpClient, CreateRequest, perform, raise_for_status,
  client for REST API, wrap upstream service, mirror of handler.
name: create-http-client
---

# Create a typed HTTP client component (mirror of an HTTP handler)

This skill is the **inverse** of [`create-http-handler`](../create-http-handler/SKILL.md).
A handler receives an HTTP request and dispatches it to typed methods; a
**client component** wraps `clients::http::Client` and exposes one typed
C++ method per OpenAPI operation. Callers (handlers, caches, other
components) get a reference to your client and call
`client.CreateOrder(req)` / `client.GetOrder(id)` — they never see URLs,
JSON, or status codes.

For the underlying HTTP client mechanics (request builder, timeouts,
retries) see the canonical
[`http_caching_tutorial.md`](http_caching_tutorial.md) and
[`http_caching_main.cpp.example`](http_caching_main.cpp.example).
For the surrounding component scaffolding see
[`create-component`](../create-component/SKILL.md).

> If you have an OpenAPI YAML schema and want **generated** code instead of
> a hand-written wrapper, use chaotic codegen
> (`userver_target_generate_openapi_client`); this skill is for the
> hand-written case.

## Trigger / Inputs

Load this skill when you need to:
- Expose a typed C++ facade for an upstream HTTP service (one method per
  endpoint).
- Mirror an existing OpenAPI schema in C++ without codegen.
- Centralize URL construction, timeouts, retries, auth headers, and
  error mapping in one place.

Inputs you need before starting:
- Base URL of the upstream (from static config).
- List of operations: HTTP method + path + request body schema + response
  body schema + error schema.
- Per-operation timeout/retry policy (or a single default).
- Auth header / token source, if any.

## 1. Register the HTTP client framework in `main.cpp`

`components::HttpClient` is NOT in `MinimalServerComponentList()`. Append
it together with the DNS component and your own client component:

```cpp
#include <userver/clients/dns/component.hpp>
#include <userver/clients/http/component.hpp>
#include <userver/clients/http/component_list.hpp>
#include <userver/components/minimal_server_component_list.hpp>
#include <userver/utils/daemon_run.hpp>

#include "src/clients/orders/component.hpp"

int main(int argc, char* argv[]) {
    const auto component_list =
        components::MinimalServerComponentList()
            .Append<your_service::clients::orders::Component>()
            .Append<clients::dns::Component>()
            .AppendComponentList(clients::http::ComponentList());
    return utils::DaemonMain(argc, argv, component_list);
}
```

## 2. Define typed schemas (one struct per operation)

Mirror exactly what the handler skill does on the server side
([`create-http-handler` §7](../create-http-handler/SKILL.md)) — same
structs, but you consume them in the opposite direction (serialize
requests, parse responses).

```cpp
// src/clients/orders/schemas.hpp
#pragma once
#include <optional>
#include <string>

namespace your_service::clients::orders {

struct CreateOrderRequest {
    std::string customer_id;
    int amount;
    std::optional<std::string> note;
};

struct CreateOrderResponse {
    std::string id;
    std::string status;
    std::string created_at;
};

struct GetOrderResponse {
    std::string id;
    std::string customer_id;
    int amount;
    std::string status;
};

// Mirror of V1Error from the handler side — we PARSE it from the upstream.
struct UpstreamError {
    std::string code;
    std::string message;
};

}  // namespace your_service::clients::orders
```

## 3. Declare the client component

```cpp
// src/clients/orders/component.hpp
#pragma once

#include <chrono>
#include <string>

#include <userver/clients/http/client.hpp>
#include <userver/components/component_base.hpp>
#include <userver/utest/using_namespace_userver.hpp>

#include "schemas.hpp"

namespace your_service::clients::orders {

class Component final : public components::ComponentBase {
public:
    static constexpr std::string_view kName = "orders-client";

    Component(const components::ComponentConfig& config,
              const components::ComponentContext& context);

    // One method per OpenAPI operation.
    CreateOrderResponse CreateOrder(const CreateOrderRequest& req) const;
    GetOrderResponse    GetOrder(const std::string& id) const;
    void                DeleteOrder(const std::string& id) const;

    static yaml_config::Schema GetStaticConfigSchema();

private:
    // Helpers
    std::string MakeUrl(std::string_view path) const;

    clients::http::Client& http_client_;
    const std::string base_url_;
    const std::chrono::milliseconds timeout_;
    const int retries_;
};

}  // namespace your_service::clients::orders
```

Key points:
- Inherit from `components::ComponentBase` (see
  [`create-component`](../create-component/SKILL.md)).
- Store `clients::http::Client&` (reference, not pointer / not by value).
  The reference is valid for the whole life of your component
  (@ref clients_from_components_lifetime).
- Public API is **typed**: no `clients::http::Response`, no raw JSON,
  no URLs leak out.

## 4. Implement the constructor

```cpp
// src/clients/orders/component.cpp
#include "component.hpp"

#include <userver/clients/http/component.hpp>
#include <userver/components/component.hpp>
#include <userver/yaml_config/merge_schemas.hpp>

namespace your_service::clients::orders {

Component::Component(const components::ComponentConfig& config,
                     const components::ComponentContext& context)
    : ComponentBase(config, context),
      http_client_(context.FindComponent<components::HttpClient>()
                          .GetHttpClient()),
      base_url_(config["base-url"].As<std::string>()),
      timeout_(config["timeout"].As<std::chrono::milliseconds>(
          std::chrono::milliseconds{500})),
      retries_(config["retries"].As<int>(2)) {}

std::string Component::MakeUrl(std::string_view path) const {
    return base_url_ + std::string{path};
}

}  // namespace your_service::clients::orders
```

The reference acquisition pattern is the canonical one from the
http_caching sample
([`main.cpp:97-100`](../../../userver/samples/http_caching/main.cpp)):

```cpp
http_client_(context.FindComponent<components::HttpClient>().GetHttpClient()),
```

## 5. Implement one method per operation

Each method:
1. Serializes the typed request → JSON (or builds query string).
2. Builds an HTTP request with `http_client_.CreateRequest()`.
3. Calls `.perform()`, then maps status code → exception or typed response.

### POST — create something

```cpp
CreateOrderResponse Component::CreateOrder(const CreateOrderRequest& req) const {
    const auto body = SerializeCreateOrderRequest(req);  // returns std::string

    auto response =
        http_client_.CreateRequest()
            .post(MakeUrl("/v1/orders"), body)
            .headers({{"Content-Type", "application/json"},
                      {"Accept",       "application/json"}})
            .retry(retries_)
            .timeout(timeout_)
            .perform();

    ThrowOnError(*response);  // maps 4xx/5xx → typed exception
    return ParseCreateOrderResponse(response->body_view());
}
```

### GET with path arg — fetch a resource

```cpp
GetOrderResponse Component::GetOrder(const std::string& id) const {
    auto response =
        http_client_.CreateRequest()
            .get(MakeUrl("/v1/orders/" + id))
            .headers({{"Accept", "application/json"}})
            .retry(retries_)
            .timeout(timeout_)
            .perform();

    ThrowOnError(*response);
    return ParseGetOrderResponse(response->body_view());
}
```

### GET with query parameters — use `http::MakeUrl`

```cpp
#include <userver/http/url.hpp>

const auto url = http::MakeUrl(MakeUrl("/v1/orders"),
                               {{"status", "open"}, {"limit", "100"}});
auto response = http_client_.CreateRequest().get(url).timeout(timeout_).perform();
```

### DELETE — no response body

```cpp
void Component::DeleteOrder(const std::string& id) const {
    auto response =
        http_client_.CreateRequest()
            .delete_method(MakeUrl("/v1/orders/" + id))
            .timeout(timeout_)
            .perform();
    ThrowOnError(*response);
}
```

The request-builder API (`get`, `post`, `put`, `patch`, `delete_method`,
`headers`, `timeout`, `retry`, `verify`, `http_version`, `data`,
`method`, `url`) lives on `clients::http::Request` — see
[`request.hpp`](../../../userver/core/include/userver/clients/http/request.hpp).

## 6. JSON parsing / serialization

Mirror the handler side ([`create-http-handler` §8](../create-http-handler/SKILL.md))
but in the opposite direction.

```cpp
// src/clients/orders/codec.cpp
#include <userver/formats/json.hpp>

namespace your_service::clients::orders {

namespace json = userver::formats::json;

std::string SerializeCreateOrderRequest(const CreateOrderRequest& req) {
    json::ValueBuilder b;
    b["customer_id"] = req.customer_id;
    b["amount"]      = req.amount;
    if (req.note) b["note"] = *req.note;
    return json::ToString(b.ExtractValue());
}

CreateOrderResponse ParseCreateOrderResponse(std::string_view body) {
    const auto v = json::FromString(body);
    return {
        .id         = v["id"].As<std::string>(),
        .status     = v["status"].As<std::string>(),
        .created_at = v["created_at"].As<std::string>(),
    };
}

GetOrderResponse ParseGetOrderResponse(std::string_view body) {
    const auto v = json::FromString(body);
    return {
        .id          = v["id"].As<std::string>(),
        .customer_id = v["customer_id"].As<std::string>(),
        .amount      = v["amount"].As<int>(),
        .status      = v["status"].As<std::string>(),
    };
}

}  // namespace your_service::clients::orders
```

## 7. Map HTTP errors to typed exceptions

Pick a domain exception type (or a small hierarchy) and convert status
codes into it. This is the mirror of the handler's "catch → set status →
return V1Error".

```cpp
class OrdersClientError : public std::runtime_error {
public:
    OrdersClientError(int status, std::string code, std::string message)
        : std::runtime_error(message),
          status_(status), code_(std::move(code)) {}
    int status() const noexcept { return status_; }
    const std::string& code() const noexcept { return code_; }
private:
    int status_;
    std::string code_;
};

void ThrowOnError(const clients::http::Response& response) {
    if (response.IsOk()) return;

    std::string code = "upstream_error";
    std::string message = "upstream returned status " +
                          std::to_string(response.status_code());
    try {
        const auto body = userver::formats::json::FromString(response.body_view());
        code    = body["code"].As<std::string>(code);
        message = body["message"].As<std::string>(message);
    } catch (const userver::formats::json::ParseException&) {
        // Non-JSON error body — keep defaults.
    }
    throw OrdersClientError(response.status_code(), std::move(code),
                            std::move(message));
}
```

Alternative for the simple case: `response->raise_for_status()` throws
`clients::http::HttpClientException` for status >= 400. Use it when you
do NOT need to inspect the upstream error body.

## 8. Static config

`GetStaticConfigSchema()`:

```cpp
yaml_config::Schema Component::GetStaticConfigSchema() {
    return yaml_config::MergeSchemas<components::ComponentBase>(R"(
type: object
description: orders HTTP client component
additionalProperties: false
properties:
    base-url:
        type: string
        description: base URL of the upstream orders service
    timeout:
        type: string
        description: per-request timeout (e.g. 500ms, 1s)
        defaultDescription: 500ms
    retries:
        type: integer
        description: number of retries on transient failures
        defaultDescription: '2'
)");
}
```

`static_config.yaml`:

```yaml
components_manager:
    components:
        orders-client:
            base-url: 'http://orders.example.com'
            timeout: 500ms
            retries: 2
        # http-client / dns-client come from clients::http::ComponentList()
        # and only need overrides for non-default settings.
```

## 9. Use the client from a handler

A handler (built with [`create-http-handler`](../create-http-handler/SKILL.md))
acquires the client reference exactly like any other component:

```cpp
class CreateOrderHandler final : public server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "handler-create-order";

    CreateOrderHandler(const components::ComponentConfig& cfg,
                       const components::ComponentContext& ctx)
        : HttpHandlerBase(cfg, ctx),
          orders_(ctx.FindComponent<your_service::clients::orders::Component>()) {}

    std::string HandleRequest(server::http::HttpRequest& request,
                              server::request::RequestContext&) const override {
        try {
            auto req = ParseCreateOrderRequest(request.RequestBody());
            auto resp = orders_.CreateOrder(req);   // typed call
            request.GetHttpResponse().SetContentType(
                userver::http::content_type::kApplicationJson);
            return ToJson(resp);
        } catch (const your_service::clients::orders::OrdersClientError& e) {
            // Map upstream failure → public API error.
            request.GetHttpResponse().SetStatus(
                e.status() >= 500 ? server::http::HttpStatus::kBadGateway
                                  : server::http::HttpStatus::kBadRequest);
            return ToJson(V1Error{e.code(), e.what(), std::nullopt});
        }
    }

private:
    your_service::clients::orders::Component& orders_;
};
```

## 10. Functional test — mock the upstream

In `config_vars.testsuite.yaml`:

```yaml
orders-base-url: $mockserver
```

and in `static_config.yaml`:

```yaml
orders-client:
    base-url: $orders-base-url
```

Then in `conftest.py`:

```python
@pytest.fixture
def mock_orders(mockserver):
    @mockserver.json_handler('/v1/orders', method='POST')
    def _create(request):
        return {'id': 'o-1', 'status': 'open',
                'created_at': '2024-01-01T00:00:00Z'}
    return _create
```

See [`http_caching_tutorial.md`](http_caching_tutorial.md) §"Functional
testing" for the full pattern.

## Checks

1. Service starts: no missing `http-client` / `dns-client`. If they fail,
   you forgot `.AppendComponentList(clients::http::ComponentList())` or
   `clients::dns::Component`.
2. From a handler test, calling the typed method actually hits the
   mockserver fixture (verify the fixture's call count > 0).
3. Upstream 4xx/5xx becomes `OrdersClientError` (or your chosen domain
   exception), NOT a raw `HttpClientException` leaking through the API.
4. Timeouts fire within `timeout` ± a small margin when the mock sleeps
   longer.
5. `X-YaTraceId` / `X-YaSpanId` are propagated to the upstream
   automatically by the tracing middleware.

## Stop conditions (ask a human)

- The upstream uses mTLS, custom CA, or rotating tokens — coordinate with
  the platform team; do NOT inline `.verify(false)` as a workaround.
- The OpenAPI schema is large or owned by another team — strongly
  consider generated code (`userver_target_generate_openapi_client`)
  instead of hand-writing every operation.
- You need streaming bodies / SSE / WebSocket — different skill.

## Troubleshooting

- **`No component with name 'http-client'`** — missing
  `.AppendComponentList(clients::http::ComponentList())`.
- **`No component with name 'dns-client'`** — add
  `clients::dns::Component`.
- **`HttpClientException: timeout was reached`** — upstream too slow or
  `timeout` too tight. Do not silently swallow.
- **`SSL peer certificate ... was not OK`** — install the proper CA
  bundle; `.verify(false)` is debug-only.
- **`CreateRequest` from a non-coroutine context** — wrap in
  `utils::Async` on the appropriate task processor.
- **`response` accessed after the shared_ptr is dropped** — keep
  `response` alive while reading `body_view()`.

## Symmetry with `create-http-handler`

| Concern              | Handler ([create-http-handler](../create-http-handler/SKILL.md)) | Client (this skill) |
|----------------------|------------------------------------------------------------------|---------------------|
| Class base           | `server::handlers::HttpHandlerBase`                              | `components::ComponentBase` |
| Direction            | Receives `HttpRequest`, returns body string                      | Builds `Request`, returns typed struct |
| Schemas              | Parse request body, build response body                          | Build request body, parse response body |
| Error shape          | Sets `HttpStatus`, returns `V1Error` JSON                        | Throws `*ClientError` carrying upstream status/code |
| Routing              | `static_config.yaml` `path:` / `method:`                         | URL & method are encoded in C++ methods |
| Static config        | `path`, `method`, `task_processor`                               | `base-url`, `timeout`, `retries` |

Keep the request/response struct definitions identical on both sides
whenever you call your own service — that is the whole point of mirroring.
