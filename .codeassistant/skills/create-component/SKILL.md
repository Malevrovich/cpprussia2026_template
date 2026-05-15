---
description: >
  Step-by-step guide to create custom userver components inheriting from
  components::ComponentBase. Includes: component class declaration, constructor
  with ComponentConfig/ComponentContext, FindComponent dependencies, static
  config schema (GetStaticConfigSchema), kHasValidate/kConfigFileMode
  specializations, registration in ComponentList, static_config.yaml entry,
  client factories returned by reference.
  Trigger: create component, add component, custom component, ComponentBase,
  ComponentConfig, ComponentContext, FindComponent, GetStaticConfigSchema,
  static config schema, register component, component list, write own component,
  client factory component, singleton component.
name: create-component
---

# Create Userver Component

Procedure for writing your own userver component. For full reference see
[`component_system.md`](component_system.md). For the canonical reference
snippets used in userver documentation see
[`userver/core/src/components/component_sample_test.hpp`](../../../userver/core/src/components/component_sample_test.hpp)
and [`userver/core/src/components/component_sample_test.cpp`](../../../userver/core/src/components/component_sample_test.cpp).

## Trigger / Inputs

Load this skill when you need to:
- Encapsulate a singleton with configuration (URL, timeouts, task processor).
- Wrap a client/factory that depends on other components (HttpClient,
  DynamicConfig, Postgres, Storages, etc.).
- Expose state/configuration to handlers via a typed accessor like
  `GetClient()` returning a reference.

Do NOT write a component if:
- The code has no static config and no dependencies on other components — a
  plain class is enough.
- You want to unit-test core logic — keep logic in a plain class, the component
  is just glue (see Rule of thumb in [`component_system.md`](component_system.md)).

Inputs you need before starting:
- Component name (string, used in static config section, e.g. `smth`,
  `my-cool-client`). Convention: kebab-case for the name in YAML,
  `kName` constexpr in C++.
- Namespace for the C++ class (e.g. `myservice::smth`).
- List of components it depends on (looked up via
  `context.FindComponent<T>()`).
- Static config fields it reads from YAML.

## 1. Create the header

File `src/<feature>/component.hpp`:

```cpp
#pragma once

#include <userver/components/component_base.hpp>
#include <userver/dynamic_config/source.hpp>
#include <userver/utest/using_namespace_userver.hpp>

namespace myservice::smth {

class Component final : public components::ComponentBase {
public:
    // Name used to refer to the component in static_config.yaml.
    static constexpr std::string_view kName = "smth";

    Component(const components::ComponentConfig& config,
              const components::ComponentContext& context);

    ~Component() final;

    int DoSomething() const;

    static yaml_config::Schema GetStaticConfigSchema();

private:
    dynamic_config::Source config_;
};

}  // namespace myservice::smth
```

Notes:
- Inherit from [`components::ComponentBase`](component_system.md). Mark the
  class `final` unless you really need inheritance.
- `kName` is `std::string_view` and is required — `ComponentList::Append<T>()`
  reads it.
- Define a destructor in the .cpp file (even if `= default`) to keep
  forward-declared dependencies happy.

## 2. Specialize traits (optional)

If your component validates its static config strictly (recommended) or is
allowed to be absent from the YAML, add specializations in the same header,
**after** the class definition:

```cpp
template <>
inline constexpr bool components::kHasValidate<myservice::smth::Component> = true;

template <>
inline constexpr auto components::kConfigFileMode<myservice::smth::Component> =
    components::ConfigFileMode::kNotRequired;  // or kRequired (default)
```

- `kHasValidate = true` forces schema validation even when
  `components_manager.static_config_validation.validate_all_components` is
  off.
- `kConfigFileMode::kNotRequired` makes the YAML section optional.

## 3. Implement the constructor

File `src/<feature>/component.cpp`:

```cpp
#include "component.hpp"

#include <userver/components/component.hpp>
#include <userver/dynamic_config/storage/component.hpp>
#include <userver/utils/async.hpp>
#include <userver/yaml_config/merge_schemas.hpp>

namespace myservice::smth {

Component::Component(const components::ComponentConfig& config,
                     const components::ComponentContext& context)
    : components::ComponentBase(config, context),
      // Dependency on another component via FindComponent<T>().
      // The returned reference outlives *this*.
      config_(context.FindComponent<components::DynamicConfig>().GetSource()) {
    // Read static config values.
    [[maybe_unused]] auto url = config["some-url"].As<std::string>();
    const auto fs_tp_name = config["fs-task-processor"].As<std::string>();

    // Run blocking work on a dedicated task processor named in config.
    auto& fs_task_processor = context.GetTaskProcessor(fs_tp_name);
    utils::Async(fs_task_processor, "smth/init", [] { /* ... */ }).Get();
}

Component::~Component() = default;

}  // namespace myservice::smth
```

Rules of construction order:
- Each component is constructed in its own task on the default task
  processor; `FindComponent<T>()` suspends until `T` is ready.
- Components are destroyed in **reverse order** of construction — references
  obtained via `FindComponent` are always valid for the lifetime of the
  current component.
- If any component fails to load, `FindComponent` throws
  `components::ComponentsLoadCancelledException` — propagate it.

## 4. Implement `GetStaticConfigSchema`

```cpp
yaml_config::Schema Component::GetStaticConfigSchema() {
    return yaml_config::MergeSchemas<components::ComponentBase>(R"(
type: object
description: smth component — does something useful
additionalProperties: false
properties:
    some-url:
        type: string
        description: url for something
        defaultDescription: http://localhost
    fs-task-processor:
        type: string
        description: name of the task processor for blocking FS syscalls
)");
}
```

Schema rules (from [`component_system.md`](component_system.md)):
- Every schema and sub-schema MUST have a `description`.
- Add `defaultDescription` whenever the property has a default value.
- `object` requires `additionalProperties` and `properties`; may have
  `required`. `array` requires `items`.
- Supported scalar types: `boolean`, `string`, `integer`, `double`.

## 5. Add a client/factory accessor (typical pattern)

If the component wraps a client, expose it via a `Get*` returning a reference.
The reference must live as long as the component:

```cpp
ClientA& GetClientA() { return client_a_; }
```

Consumers (handlers, other components) call
`context.FindComponent<Component>().GetClientA()` once in their constructor
and store the reference.

## 6. Register in `ComponentList`

In `main.cpp`:

```cpp
#include <userver/components/minimal_server_component_list.hpp>
#include <userver/utils/daemon_run.hpp>

#include <src/smth/component.hpp>

int main(int argc, char* argv[]) {
    const auto component_list = components::MinimalServerComponentList()
        .Append<myservice::smth::Component>();
    return utils::DaemonMain(argc, argv, component_list);
}
```

For non-server daemons use `components::Run` / `components::RunOnce` as
described in [`component_system.md`](component_system.md).

## 7. Add the section in `static_config.yaml`

```yaml
components_manager:
    components:
        smth:
            some-url: http://example.com
            fs-task-processor: fs-task-processor
```

If you specialized `kConfigFileMode::kNotRequired` the section may be omitted.
The framework-wide flag `load-enabled: false` disables loading of any
component without removing the section.

## 8. Build and verify

Checks:
1. Service builds: header included only where needed; destructor defined.
2. Service starts without
   `InvalidConfigException` / schema errors — fix missing `description` or
   typos in property names.
3. Component is constructed: add a `LOG_INFO()` in the constructor and
   confirm it appears in logs on startup.
4. Consumers can resolve it via `FindComponent<Component>()` without a
   `ComponentsLoadCancelledException`.

## Troubleshooting

- **`No component with name 'smth'`** — section missing in YAML, `kName`
  mismatch, or component not `Append`ed to the `ComponentList`.
- **`Failed to parse static config ... additional property ... is not allowed`**
  — schema has `additionalProperties: false` and YAML uses an unknown key,
  or the schema is missing the property. Update the schema or YAML.
- **`Cyclic dependency between components`** — two components call
  `FindComponent` on each other in their constructors. Break the cycle: do
  the lookup lazily, or split into two components.
- **`ComponentsLoadCancelledException`** — some other component failed to
  load; check earlier errors in the log, do not catch and swallow this
  exception.
- **Component constructed twice / not a singleton** — you tried to
  instantiate the class directly. Components are managed exclusively by the
  framework; never `new` them.

## Stop conditions (ask a human)

- The desired behavior requires per-request state — components are
  singletons, this likely belongs in a handler or `RequestContext`.
- Static config needs a type not supported by the schema language (custom
  variants, oneOf, etc.).
- You hit a real cyclic dependency that cannot be broken locally.

## Examples in the userver tree

Browse `userver/samples/*` for working components, e.g.:
- `userver/samples/hello_service` — minimal HTTP service with a custom
  handler component.
- `userver/samples/postgres_service` — component wrapping a PG client.
- `userver/samples/http_caching` — component as a cache.

Canonical doc snippets live in
[`userver/core/src/components/component_sample_test.hpp`](../../../userver/core/src/components/component_sample_test.hpp)
and [`userver/core/src/components/component_sample_test.cpp`](../../../userver/core/src/components/component_sample_test.cpp).
