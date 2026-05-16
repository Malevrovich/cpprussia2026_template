---
description: >
  Step-by-step guide to create HTTP server middlewares in userver services.
  Includes: HttpMiddlewareBase implementation, HttpMiddlewareFactoryBase, SimpleHttpMiddlewareFactory,
  per-handler middleware configuration, server-wide pipeline builder, custom handler pipeline builder,
  response header injection, exception-safe ordering relative to ExceptionsHandling middleware.
  Trigger: create middleware, add middleware, http middleware, request interceptor, response interceptor,
  middleware pipeline, pipeline builder, HandlerPipelineBuilder, HttpMiddlewareBase, auth middleware,
  logging middleware, custom header middleware, modify request before handler, modify response after handler.
name: create-middleware
---

# Create HTTP Server Middleware in Userver

Complete procedure for adding a custom HTTP middleware to a userver service.

A middleware sits in the request/response pipeline of `components::Server` and can:
- Inspect / mutate the incoming `server::http::HttpRequest` before the handler runs.
- Mutate the response after the downstream pipeline ran (including the handler).
- Short-circuit the pipeline (skip `Next(...)` and produce its own response).
- Catch / let through exceptions from downstream.

Key concept: a **Middleware is a Client, not a Component**. There are `M × H` middleware instances (one per handler per middleware). Dependencies are injected by a paired **MiddlewareFactory**, which IS a component (singleton).

Full upstream doc lives next to this skill: see [`http_server_middlewares.md`](http_server_middlewares.md).
Full sample lives upstream in `userver/samples/http_middleware_service/`; key snippets are inlined below in [`middleware_example.cpp`](middleware_example.cpp) and [`static_config_example.yaml`](static_config_example.yaml).

---

## When to use a middleware (vs a base handler)

Use a middleware when the logic is **cross-cutting** and applies to many handlers:
- Auth / authz checks before the handler.
- Tracing / metrics / access logs.
- Adding response headers (CORS, custom server headers).
- Rate limiting, decompression, request body size limits.
- Catching specific exceptions and translating them into HTTP responses.

Do NOT use a middleware for per-handler business logic — that belongs in the handler itself.

---

## 1. Pick the right base interface

There are two basic interfaces:

| Need | Use |
|------|-----|
| Stateless middleware, no per-handler config, just `Next(...)` + small logic | `HttpMiddlewareBase` + `SimpleHttpMiddlewareFactory<T>` |
| Per-handler config OR factory needs to inject dependencies from `ComponentContext` | `HttpMiddlewareBase` + custom subclass of `HttpMiddlewareFactoryBase` |

The middleware itself always derives from `server::middlewares::HttpMiddlewareBase` and overrides:

```cpp
void HandleRequest(server::http::HttpRequest& request,
                   server::request::RequestContext& context) const override;
```

Inside `HandleRequest` you call `Next(request, context)` to pass execution to the next middleware in the pipeline (or skip it to short-circuit).

---

## 2. Minimal noop middleware (just passes through)

```cpp
#include <userver/server/middlewares/http_middleware_base.hpp>

class NoopMiddleware final : public server::middlewares::HttpMiddlewareBase {
    void HandleRequest(server::http::HttpRequest& request,
                       server::request::RequestContext& context) const override {
        Next(request, context);
    }
};

class NoopMiddlewareFactory final : public server::middlewares::HttpMiddlewareFactoryBase {
public:
    static constexpr std::string_view kName{"noop-middleware"};
    using HttpMiddlewareFactoryBase::HttpMiddlewareFactoryBase;

private:
    std::unique_ptr<server::middlewares::HttpMiddlewareBase> Create(
        const server::handlers::HttpHandlerBase&,
        yaml_config::YamlConfig) const override {
        return std::make_unique<NoopMiddleware>();
    }
};
```

---

## 3. Middleware with `SimpleHttpMiddlewareFactory` shortcut

If the middleware just needs a const-ref to the handler (or nothing) and no per-handler YAML config, you can avoid hand-writing the factory:

```cpp
class SomeServerMiddleware final : public server::middlewares::HttpMiddlewareBase {
public:
    // This becomes the component name for SimpleHttpMiddlewareFactory
    static constexpr std::string_view kName{"server-middleware"};

    explicit SomeServerMiddleware(const server::handlers::HttpHandlerBase&) {}

private:
    void HandleRequest(server::http::HttpRequest& request,
                       server::request::RequestContext& context) const override {
        Next(request, context);
        request.GetHttpResponse().SetHeader(kCustomServerHeader, "1");
    }

    static constexpr http::headers::PredefinedHeader kCustomServerHeader{
        "X-Some-Server-Header"};
};

using SomeServerMiddlewareFactory =
    server::middlewares::SimpleHttpMiddlewareFactory<SomeServerMiddleware>;
```

Requirements for `SimpleHttpMiddlewareFactory<T>`:
- `T::kName` static member of type `std::string_view`.
- `T` constructible from `const server::handlers::HttpHandlerBase&`.

---

## 4. Configurable middleware (per-handler YAML config)

When you need values from the handler's `middlewares:` section in `static_config.yaml`, implement the factory yourself and parse the YAML in the middleware constructor.

```cpp
class SomeHandlerMiddleware final : public server::middlewares::HttpMiddlewareBase {
public:
    static constexpr std::string_view kName{"handler-middleware"};

    SomeHandlerMiddleware(const server::handlers::HttpHandlerBase&,
                          yaml_config::YamlConfig middleware_config)
        : header_value_{middleware_config["header-value"].As<std::string>()} {}

private:
    void HandleRequest(server::http::HttpRequest& request,
                       server::request::RequestContext& context) const override {
        // ScopeGuard guarantees the header is set even if downstream throws
        // (subject to ExceptionsHandling middleware ordering, see Caveats).
        const utils::ScopeGuard set_header_scope{[this, &request] {
            request.GetHttpResponse().SetHeader(kCustomHandlerHeader, header_value_);
        }};
        Next(request, context);
    }

    static constexpr http::headers::PredefinedHeader kCustomHandlerHeader{
        "X-Some-Handler-Header"};
    const std::string header_value_;
};

class SomeHandlerMiddlewareFactory final
    : public server::middlewares::HttpMiddlewareFactoryBase {
public:
    static constexpr std::string_view kName{SomeHandlerMiddleware::kName};
    using HttpMiddlewareFactoryBase::HttpMiddlewareFactoryBase;

private:
    std::unique_ptr<server::middlewares::HttpMiddlewareBase> Create(
        const server::handlers::HttpHandlerBase& handler,
        yaml_config::YamlConfig middleware_config) const override {
        return std::make_unique<SomeHandlerMiddleware>(handler,
                                                       std::move(middleware_config));
    }

    yaml_config::Schema GetMiddlewareConfigSchema() const override {
        return formats::yaml::FromString(R"(
type: object
description: Config for this particular middleware
additionalProperties: false
properties:
    header-value:
        type: string
        description: header value to set for responses
)").As<yaml_config::Schema>();
    }
};
```

Always declare a schema in `GetMiddlewareConfigSchema()` — userver will reject unknown options at startup, which catches typos in handler configs.

---

## 5. Middleware factory with component dependencies (global config)

If the middleware needs a shared resource (DB client, cache, rate limiter, dynamic config), look it up in the **factory** constructor (it IS a component) and pass it to every created middleware instance.

```cpp
class AuthMiddlewareFactory final
    : public server::middlewares::HttpMiddlewareFactoryBase {
public:
    static constexpr std::string_view kName{"auth-middleware"};

    AuthMiddlewareFactory(const components::ComponentConfig& config,
                          const components::ComponentContext& context)
        : HttpMiddlewareFactoryBase(config, context),
          auth_client_{context.FindComponent<MyAuthClient>()} {}

private:
    std::unique_ptr<server::middlewares::HttpMiddlewareBase> Create(
        const server::handlers::HttpHandlerBase& handler,
        yaml_config::YamlConfig cfg) const override {
        return std::make_unique<AuthMiddleware>(handler, auth_client_, std::move(cfg));
    }

    MyAuthClient& auth_client_;
};
```

Rule of thumb: prefer factory-level (global) config over per-handler config to avoid copy-pasta in `static_config.yaml`. Mix both when some knob really must vary per handler.

---

## 6. Register the factory in the component list

```cpp
int main(int argc, char* argv[]) {
    const auto component_list = components::MinimalServerComponentList()
        .Append<Handler>("handler")
        .Append<SomeServerMiddlewareFactory>()    // SimpleHttpMiddlewareFactory alias
        .Append<SomeHandlerMiddlewareFactory>()
        .Append<NoopMiddlewareFactory>();
    return utils::DaemonMain(argc, argv, component_list);
}
```

Only **factories** are appended; middleware instances are created by factories per handler.

---

## 7. Add factories to `static_config.yaml`

Each factory is a normal component and needs an entry, even if empty:

```yaml
components_manager:
    components:
        noop-middleware: {}
        server-middleware: {}
        handler-middleware: {}
```

---

## 8. Add the middleware to the pipeline

Just registering a factory is NOT enough — userver won't use it until you put its name into a pipeline.

### 8a. Server-wide (applies to every handler)

Use the built-in `default-server-middleware-pipeline-builder` and `append:` your middleware to the default pipeline:

```yaml
components_manager:
    components:
        default-server-middleware-pipeline-builder:
            append:
              - server-middleware
```

For more complex transformations, subclass `server::middlewares::PipelineBuilder`, override `BuildPipeline`, register it as a component, and wire it via:

```yaml
server:
    middleware-pipeline-builder: custom-pipeline-builder
```

### 8b. Per-handler pipeline (applies only to that handler)

Subclass `server::middlewares::HandlerPipelineBuilder`:

```cpp
class CustomHandlerPipelineBuilder final
    : public server::middlewares::HandlerPipelineBuilder {
public:
    using HandlerPipelineBuilder::HandlerPipelineBuilder;

    server::middlewares::MiddlewaresList BuildPipeline(
        server::middlewares::MiddlewaresList server_middleware_pipeline) const override {
        auto& pipeline = server_middleware_pipeline;
        pipeline.emplace_back(SomeHandlerMiddleware::kName);
        pipeline.emplace_back(NoopMiddlewareFactory::kName);
        return pipeline;
    }
};
```

Append it to the component list and wire it from the handler's static config:

```yaml
custom-handler-pipeline-builder: {}

handler-with-custom-middlewares:
    path: /custom-hello
    method: GET
    task_processor: main-task-processor
    middlewares:
        pipeline-builder: custom-handler-pipeline-builder
        handler-middleware:
            header-value: some_value
```

Per-handler middleware config (e.g. `header-value`) is placed under `middlewares.<middleware-name>` and reaches the middleware constructor as `yaml_config::YamlConfig`.

---

## 9. Caveats (read before shipping)

1. **Never throw from a middleware.** Letting a downstream exception propagate is fine; throwing your own is not — it can produce malformed responses and break HTTP semantics.
2. **Response-mutating middlewares must run BEFORE `ExceptionsHandling`** in the pipeline, otherwise their mutations may be overwritten when `ExceptionsHandling` rewrites the response on a downstream throw. Alternatively, handle the downstream exception yourself.
3. Mutating the request AFTER `Next(...)` is meaningless (the handler already ran). Pre-handler logic goes before `Next`, post-handler logic goes after `Next`, response cleanup is best done via `utils::ScopeGuard` for exception-safety.
4. Default userver pipeline already provides tracing, decompression, rate limiting, exception handling, deadline propagation, etc. Don't reimplement; just `append:` your stuff.
5. A middleware is **not** a component — do NOT call `context.FindComponent<...>()` from inside the middleware. Do the lookup in the factory and inject.

---

## 10. Checklist before merging

- [ ] Class derives from `server::middlewares::HttpMiddlewareBase`.
- [ ] `HandleRequest` calls `Next(request, context)` exactly once (unless intentionally short-circuiting).
- [ ] `kName` constant matches the component key in `static_config.yaml`.
- [ ] Paired factory exists and is `Append<>`ed in `main.cpp`.
- [ ] Factory entry exists in `static_config.yaml` (`my-middleware: {}` at minimum).
- [ ] Middleware name is added to a pipeline (server-wide `append:` or a `HandlerPipelineBuilder`).
- [ ] If using per-handler config: `GetMiddlewareConfigSchema()` is implemented and lists all options.
- [ ] No `throw` inside the middleware body.
- [ ] Response-modifying middleware is placed before `ExceptionsHandling` (or handles exceptions itself).
- [ ] Functional test asserts the side effect (header added, request rejected, etc.).

---

## Troubleshooting

**Factory created but middleware never runs.**
You forgot to add the middleware name to a pipeline (`default-server-middleware-pipeline-builder.append` or a `HandlerPipelineBuilder`).

**Service fails to start with "unknown component".**
Either the factory isn't `Append<>`ed in `main.cpp`, or there's no `my-middleware: {}` entry in `static_config.yaml`.

**Service fails to start with "unknown property" in middleware config.**
Add the property to the schema returned by `GetMiddlewareConfigSchema()`.

**My response header disappears when the handler throws.**
The default `ExceptionsHandling` middleware overwrote the response. Move your middleware earlier in the pipeline, or use `utils::ScopeGuard` AND ensure ordering, or catch the exception yourself inside `HandleRequest`.

**Need access to a DB client / shared resource inside middleware.**
Inject it via the factory constructor (`context.FindComponent<...>()`) and pass to each middleware instance in `Create()`. Don't try to find components from `HandleRequest`.
