---
description: >
  Step-by-step guide to write functional and unit tests for a userver HTTP handler
  exposed by your service. Includes: pytest_userver testsuite setup, service_client
  fixture, request/response assertions, headers/query/body checks, status codes,
  GET/POST/PUT/DELETE/HEAD, unit tests for business logic with UTEST, CMake
  integration via userver_testsuite_add_simple.
  Trigger: test http handler, functional test, testsuite, service_client,
  test endpoint, test rest, write pytest for handler, UTEST handler,
  pytest_userver, test response status, test request headers.
name: testing-http-handler
---

# Testing a userver HTTP Handler **MANDATORY**

Use this procedure when you need to verify a `server::handlers::HttpHandlerBase`
subclass that lives in your own service. Userver provides two complementary
layers: **functional tests** (real binary, real HTTP, pytest via `service_client`)
and **unit tests** (pure C++ logic, no HTTP, `UTEST`). Cover business logic with
unit tests; cover wiring/contracts/headers/status codes with functional tests.

All examples below are verbatim from the upstream `samples/hello_service`
reference service in the official userver documentation
([`hello_service.md`](../../../userver/scripts/docs/en/userver/tutorial/hello_service.md:1),
[`functional_testing.md`](../../../userver/scripts/docs/en/userver/functional_testing.md:1),
[`testing.md`](../../../userver/scripts/docs/en/userver/testing.md:1)).

---

## 1. Reference: the handler under test

`samples/hello_service` replies to `GET/POST /hello` with `Hello, <name>!\n`.

### 1.1 Pure logic (testable without HTTP)

[`samples/hello_service/src/say_hello.hpp`](../../../userver/samples/hello_service/src/say_hello.hpp:1):

```cpp
#pragma once

#include <string>
#include <string_view>

namespace samples::hello {

std::string SayHelloTo(std::string_view name);

}  // namespace samples::hello
```

[`samples/hello_service/src/say_hello.cpp`](../../../userver/samples/hello_service/src/say_hello.cpp:1):

```cpp
#include "say_hello.hpp"

#include <fmt/format.h>

namespace samples::hello {

std::string SayHelloTo(std::string_view name) {
    if (name.empty()) {
        name = "unknown user";
    }

    return fmt::format("Hello, {}!\n", name);
}

}  // namespace samples::hello
```

### 1.2 Thin handler that forwards to the logic

[`samples/hello_service/src/hello_handler.hpp`](../../../userver/samples/hello_service/src/hello_handler.hpp:1):

```cpp
#pragma once

#include <userver/components/component_list.hpp>
#include <userver/server/handlers/http_handler_base.hpp>

// Note: this is for the purposes of tests/samples only
#include <userver/utest/using_namespace_userver.hpp>

namespace samples::hello {

class HelloHandler final : public server::handlers::HttpHandlerBase {
public:
    // `kName` is used as the component name in static config
    static constexpr std::string_view kName = "handler-hello-sample";

    // Component is valid after construction and is able to accept requests
    using HttpHandlerBase::HttpHandlerBase;

    std::string HandleRequest(server::http::HttpRequest& request, server::request::RequestContext&) const override;
};

}  // namespace samples::hello
```

[`samples/hello_service/src/hello_handler.cpp`](../../../userver/samples/hello_service/src/hello_handler.cpp:1):

```cpp
#include "hello_handler.hpp"

#include "say_hello.hpp"

namespace samples::hello {

std::string HelloHandler::HandleRequest(
    server::http::HttpRequest& request,
    server::request::RequestContext& /*request_context*/
) const {
    request.GetHttpResponse().SetContentType(http::content_type::kTextPlain);
    return samples::hello::SayHelloTo(request.GetArg("name"));
}

}  // namespace samples::hello
```

### 1.3 Static config

[`samples/hello_service/static_config.yaml`](../../../userver/samples/hello_service/static_config.yaml:1):

```yaml
# yaml
components_manager:
    task_processors:
        main-task-processor:
            worker_threads: 4
        fs-task-processor:
            worker_threads: 1

    components:
        server:
            listener:
                port: 8080
        logging:
            loggers:
                default:
                    file_path: '@stderr'
                    level: debug
                    overflow_behavior: discard

        handler-hello-sample:
            path: /hello
            method: GET,POST
```

For a service that also wants testsuite extras (cache invalidation, testpoints,
mocked time…) you additionally need `testsuite-support` and `tests-control`
components — pattern from
[`samples/http_caching/static_config.yaml`](../../../userver/samples/http_caching/static_config.yaml:14):

```yaml
testsuite-support:
tests-control:
    load-enabled: $testsuite-enabled    # MUST stay false in production
    path: /tests/{action}
    method: POST
    task_processor: main-task-processor
```

---

## 2. Functional test (pytest + `service_client`)

### 2.1 Layout (matches `userver_testsuite_add_simple()`)

```
my_service/
├── CMakeLists.txt
├── configs/
│   └── static_config.yaml
├── src/
│   └── *.{hpp,cpp}
└── testsuite/
    ├── conftest.py
    └── test_*.py
```

### 2.2 conftest.py — register the pytest plugin

[`samples/hello_service/testsuite/conftest.py`](../../../userver/samples/hello_service/testsuite/conftest.py:1):

```python
# /// [registration]
# Adding a plugin from userver/testsuite/pytest_plugins/
pytest_plugins = ['pytest_userver.plugins.core']
# /// [registration]
```

The plugin name MUST match the CMake target your service links against
(see the table in
[`functional_testing.md`](../../../userver/scripts/docs/en/userver/functional_testing.md:175)).
E.g. `userver::postgresql` → add `pytest_userver.plugins.postgresql`.

### 2.3 The test itself

[`samples/hello_service/testsuite/test_hello.py`](../../../userver/samples/hello_service/testsuite/test_hello.py:1):

```python
# /// [Functional test]
async def test_hello_base(service_client):
    response = await service_client.get('/hello')
    assert response.status == 200
    assert 'text/plain' in response.headers['Content-Type']
    assert response.text == 'Hello, unknown user!\n'
    assert 'X-RequestId' not in response.headers.keys(), 'Unexpected header'

    response = await service_client.get('/hello', params={'name': 'userver'})
    assert response.status == 200
    assert 'text/plain' in response.headers['Content-Type']
    assert response.text == 'Hello, userver!\n'
    # /// [Functional test]


async def test_hello_head(service_client):
    response = await service_client.request('HEAD', '/hello')
    assert response.status == 200
    assert 'text/plain' in response.headers['Content-Type']
    assert response.text == ''
    assert 'X-RequestId' not in response.headers.keys(), 'Unexpected header'


async def test_wrong_method(service_client):
    response = await service_client.request('KEK', '/hello')
    assert response.status == 400
    assert response.text == 'bad request'
    assert 'X-YaRequestId' not in response.headers.keys(), 'Unexpected header'
```

`service_client` is a `pytest_userver.client.Client`; responses are
`testsuite.utils.http.ClientResponse` (`.status`, `.headers`, `.text`,
`.json()`, `.content`). The bare minimum body of a test (from
[`samples/testsuite-support/tests/test_ping.py`](../../../userver/samples/testsuite-support/tests/test_ping.py:1)):

```python
import pytest_userver.client
import testsuite.utils.http

# /// [service_client]
async def test_ping(service_client: pytest_userver.client.Client):
    response: testsuite.utils.http.ClientResponse = await service_client.get('/ping')
    assert response.status == 200
    # /// [service_client]
```

Common request helpers on `service_client`:
- `await service_client.get(path, params=..., headers=...)`
- `await service_client.post(path, json=..., data=..., params=..., headers=...)`
- `await service_client.put(path, json=...)`
- `await service_client.delete(path)`
- `await service_client.request(method, path, ...)` — arbitrary verbs

### 2.4 POST with JSON body and headers

```python
async def test_create_user(service_client):
    response = await service_client.post(
        '/v1/users',
        json={'login': 'alice', 'email': 'alice@example.com'},
        headers={'X-Request-Id': 'test-1'},
    )
    assert response.status == 201
    body = response.json()
    assert body['login'] == 'alice'
    assert 'id' in body
```

### 2.5 Per-test state hooks

On the first `service_client` call in a test, userver implicitly invalidates
caches, resets mocked time, re-registers testpoints. Force a reset manually
when needed:

```python
await service_client.invalidate_caches()
await service_client.update_server_state()
```

---

## 3. CMake wiring

OBJECT-library pattern from
[`samples/hello_service/CMakeLists.txt`](../../../userver/samples/hello_service/CMakeLists.txt:1)
so logic compiles once and is shared by the binary, unit tests, and benchmarks:

```cmake
cmake_minimum_required(VERSION 3.14)
project(userver-samples-hello_service CXX)

# /// [find_userver]
find_package(
    userver
    COMPONENTS core
    REQUIRED
)
# /// [find_userver]

# /// [objects]
add_library(${PROJECT_NAME}_objs OBJECT
    src/say_hello.hpp src/say_hello.cpp
    src/hello_handler.hpp src/hello_handler.cpp
)
target_link_libraries(${PROJECT_NAME}_objs userver::core)
target_include_directories(${PROJECT_NAME}_objs PUBLIC src)
# /// [objects]

# /// [executable]
add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} ${PROJECT_NAME}_objs)
# /// [executable]

# /// [unittests]
add_executable(${PROJECT_NAME}-unittest unittests/say_hello_test.cpp)
target_link_libraries(${PROJECT_NAME}-unittest ${PROJECT_NAME}_objs userver::utest)
add_google_tests(${PROJECT_NAME}-unittest)
# /// [unittests]

# /// [testsuite]
userver_testsuite_add_simple()
# /// [testsuite]
```

`userver_testsuite_add_simple()` creates a `testsuite-<target>` ctest target,
a `start-<target>` target, a per-target Python venv with userver's testsuite
deps, and the `runtests-<target>` runner script
([`functional_testing.md`](../../../userver/scripts/docs/en/userver/functional_testing.md:71)).

---

## 4. Unit test for handler business logic

Functional tests are slow. Extract pure logic out of `HandleRequest` (see §1.1)
and test it directly.

[`samples/hello_service/unittests/say_hello_test.cpp`](../../../userver/samples/hello_service/unittests/say_hello_test.cpp:1):

```cpp
/// [Unit test]
#include "say_hello.hpp"

#include <userver/utest/utest.hpp>

UTEST(SayHelloTo, Basic) {
    EXPECT_EQ(samples::hello::SayHelloTo("Developer"), "Hello, Developer!\n");
    EXPECT_EQ(samples::hello::SayHelloTo({}), "Hello, unknown user!\n");
}
/// [Unit test]
```

Use `UTEST` (or `UTEST_F`, `UTEST_P`, `UTEST_MT`) whenever the code under test
touches the coroutine engine (tasks, mutexes, `engine::SingleConsumerEvent`,
`utils::Async`…). For pure logic `TEST` is enough, but `UTEST` is always safe.
The full list of `U`-macros is in
[`testing.md`](../../../userver/scripts/docs/en/userver/testing.md:33).

### Exception assertions with proper diagnostics

```cpp
#include <userver/utest/assert_macros.hpp>

UTEST(MyHandler, ThrowsOnBadInput) {
    UEXPECT_THROW_MSG(
        MyParser::Parse("garbage"),
        std::runtime_error,
        "invalid format"
    );
}
```

Available: `UEXPECT_THROW_MSG`, `UASSERT_THROW_MSG`, `UEXPECT_THROW`,
`UASSERT_THROW`, `UEXPECT_NO_THROW`, `UASSERT_NO_THROW`
([`testing.md`](../../../userver/scripts/docs/en/userver/testing.md:78)).

---

## 5. Running the tests

```bash
# All tests through ctest:
ctest -V -R testsuite-<service-target>      # functional
ctest -V -R <service-target>-unittest       # unit

# Direct functional runner:
./build/<path>/runtests-<service-target> -vvs -k test_hello_base

# Direct unit-test binary:
./build/<path>/<service-target>-unittest --gtest_filter=*SayHelloTo*
```

Useful pytest flags (via `PYTEST_ARGS` to `userver_testsuite_add` or on the
command line): `-v`, `-s`, `-x`, `--service-logs-pretty`,
`--service-log-level=debug`, `--service-wait` (pauses so you can attach gdb;
testsuite prints the exact gdb command). Full list:
[`functional_testing.md`](../../../userver/scripts/docs/en/userver/functional_testing.md:55).

---

## 6. Checks before declaring done

1. `ctest -R testsuite-<target>` and `ctest -R <target>-unittest` both pass.
2. Functional test covers at least: happy path, validation error, wrong method.
3. Status code, `Content-Type` and body are asserted explicitly.
4. Pure business logic is unit-tested (`UTEST`/`TEST`), not only via HTTP.
5. `tests-control` is gated by `load-enabled: $testsuite-enabled` and is **not**
   loaded in production.

---

## 7. Troubleshooting

- **`fixture 'service_client' not found`** — `pytest_plugins = ['pytest_userver.plugins.core']`
  is missing from `conftest.py`, or you used the wrong plugin row for your
  CMake target.
- **Service hangs on first request** — caches are warming up. Either wait, or
  patch their configs through `USERVER_CONFIG_HOOKS`
  ([`functional_testing.md`](../../../userver/scripts/docs/en/userver/functional_testing.md:225)).
- **Test sees old data after a PATCH/PUT** — call
  `await service_client.invalidate_caches()`.
- **Need to attach gdb to the service during a test** — pass `--service-wait`;
  testsuite prints the command to launch the binary manually.
- **UTEST hangs / deadlocks** — the test uses synchronization primitives but
  was written with `TEST` instead of `UTEST`. Switch the macro. For race
  detection use `UTEST_MT(suite, name, thread_count)`.
- **DEATH-test instability** — use `UTEST_DEATH`; it configures gtest for the
  coroutine runtime.

---

## 8. When to stop and ask

- The handler depends on a database/queue/external system whose test setup is
  not yet wired — finish that plugin setup first.
- The endpoint requires auth — clarify whether tests pass real tokens or
  whether auth must be mocked at handler level.
- The handler reaches a third-party HTTP service — that case is covered by the
  separate skill for **mocking external HTTP clients in testsuite** (mockserver
  / `mockserver.json_handler` workflow, `USERVER_CONFIG_HOOKS` for URL
  rewriting, simulating timeouts/network errors for outbound calls).
