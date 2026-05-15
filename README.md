# Мастер‑класс: Разработка микросервисов на userver

## Задание

Перед вами проект мессенджера с готовым фронтендом, но без реализованного бекенда.  
**Ваша задача** — создать шесть микросервисов на фреймворке **userver**, которые вместе обеспечат работу мессенджера.

### Что нужно сделать

1. **Изучите структуру проекта** (описана ниже).
2. **Соберите фронтенд** (инструкция в разделе «Подготовка»).
3. **Реализуйте шесть микросервисов** согласно документации в папке `docs/`.
4. **Интегрируйте каждый сервис** в общую систему (Docker Compose, Nginx, CMake).
5. **Запустите весь стек** и убедитесь, что мессенджер работает.

---

## Структура проекта

```
cpprussia2026_template/
├── backend/                    # Директория для микросервисов
│   ├── CMakeLists.txt         # CMake‑конфигурация каждого сервиса (внутри поддиректорий)
│   └── Dockerfile             # Docker‑образ для бекенда
├── frontend/                  # Готовый фронтенд на Vue.js
│   ├── src/
│   ├── package.json
│   └── Dockerfile
├── docs/                      # Технические задания на каждый сервис
├── nginx/                     # Конфигурация Nginx (обратный прокси)
├── docker-compose.yml         # Docker Compose для запуска всего стека
└── README.md                  # Это задание
```

---

## Подготовка

### 1. Сборка фронтенда

Фронтенд необходимо собрать перед запуском в Docker.

```bash
cd frontend
npm install
npm run build
```

После успешной сборки в папке `frontend/dist/` появятся статические файлы, которые будет раздавать Nginx.

### 2. Запуск инфраструктуры

Вы можете запускать сервисы по мере готовности. Например, после реализации `auth_service` вы можете раскомментировать только его секции в конфигурационных файлах и запустить стек — фронтенд будет работать с этим сервисом, а фичи, требующие других сервисов, временно не будут доступны.

Когда один или несколько сервисов готовы и соответствующие секции в конфигурационных файлах раскомментированы, выполните:

```bash
sudo docker compose build
sudo docker compose up -d
```

**Важно:** Docker Compose не собирает ни фронтенд, ни бекенд‑сервисы автоматически. Вы должны заранее:
- Собрать фронтенд (`npm run build`)
- Убедиться, что реализованные сервисы скомпилированы
- Раскомментировать секции только тех сервисов, которые уже готовы, в `docker-compose.yml` и `nginx/nginx.conf`

Фронтенд будет доступен по адресу **http://localhost:80**.

---

## Список сервисов

Вам предстоит реализовать шесть микросервисов. Каждый сервис описан в отдельном файле в папке `docs/`.

| № | Сервис | Назначение | Документация |
|---|--------|------------|--------------|
| 1 | **auth_service** | Аутентификация и авторизация | [`docs/auth-service.md`](docs/auth-service.md) |
| 2 | **status_service** | Сервис пользовательских статусов | [`docs/status-service.md`](docs/status-service.md) |
| 3 | **messaging_service** | Сервис работы с каналами и сообщениями | [`docs/messaging-service.md`](docs/messaging-service.md) |
| 4 | **reactions_service** | Сервис реакций (лайков/дизлайков) на сообщения | [`docs/reactions-service.md`](docs/reactions-service.md) |
| 5 | **notifications_service** | Сервис уведомлений о сообщениях в рамках каналов | [`docs/notifications-service.md`](docs/notifications-service.md) |
| 6 | **files_service** | Сервис передачи файлов | [`docs/files-service.md`](docs/files-service.md) |

**Примечание:** Если вы решите изменить имя какого‑либо сервиса, не забудьте обновить его название в `docker-compose.yml` и `nginx/nginx.conf`.

**Рекомендация:** Начните реализацию с сервиса `auth_service`, так как он обеспечивает аутентификацию и авторизацию, необходимые для работы фронтенда и других сервисов. После его готовности вы сможете тестировать логин и регистрацию, а затем постепенно добавлять остальные сервисы.

---

## Порты сервисов

Конфигурация Nginx (`nginx/nginx.conf`) определяет порты, на которых должны работать микросервисы:

| Сервис | Внутренний порт (в Docker) | Публичный путь (через Nginx) |
|--------|----------------------------|-------------------------------|
| **auth_service** | 8001 | `http://localhost/api/auth/` |
| **messaging_service** | 8002 | `http://localhost/api/messaging/` |
| **notifications_service** | 8003 | `http://localhost/api/notifications/` |
| **files_service** | 8004 | `http://localhost/api/files/` |
| **reactions_service** | 8005 | `http://localhost/api/reactions/` |
| **status_service** | 8006 | `http://localhost/api/status/` |
| **frontend** | 3000 | `http://localhost/` |

**Важно:**
- Nginx слушает на порту **80** и проксирует запросы к соответствующим сервисам.
- Внутренние порты используются для коммуникации между контейнерами Docker.
- При реализации сервиса убедитесь, что в его `config_vars.yaml` указан правильный порт (например, `server_port: 8001` для auth_service).
- Комментирование/раскомментирование блоков в `nginx/nginx.conf` управляет доступностью сервисов.

---

## Сборка микросервисов

Перед запуском сервисов необходимо собрать их бинарные файлы. Каждый сервис компилируется независимо в своей директории.

Для сборки конкретного сервиса перейдите в его директорию и выполните:

```bash
cd backend/<service_name>
make build-release
```

Бинарник появится в `backend/<service_name>/build-release/<service_name>`.

Docker Compose будет использовать этот бинарник и конфигурационные файлы из `backend/<service_name>/configs/`.

**Важно:** После каждой пересборки сервиса необходимо пересобрать Docker‑образ, чтобы изменения попали в контейнер. Для этого выполните:

```bash
sudo docker compose build <service-name>
```

Или пересоберите все образы:

```bash
sudo docker compose build
```

---

## Пошаговая инструкция по созданию сервиса

Для каждого сервиса выполните следующие шаги:

### Шаг 1. Создание заготовки сервиса

Воспользуйтесь утилитой `userver-create-service`:

```bash
userver-create-service backend/название_сервиса
```

Например, для сервиса аутентификации:

```bash
userver-create-service backend/auth_service
```

### Шаг 2. Отключение stack‑usage monitor (обязательно для Docker)

В файле `configs/static_config.yaml` внутри директории сервиса добавьте в секцию `components_manager`:

```yaml
coro_pool:
    stack_usage_monitor_enabled: false
```

Без этого настройки в Docker‑контейнере будет возникать ошибка инициализации.

### Шаг 3. Сборка сервиса

Перейдите в директорию сервиса и выполните сборку:

```bash
cd backend/<service_name>
make build-release
```

Убедитесь, что бинарник создан в `backend/<service_name>/build-release/<service_name>`.

### Шаг 4. Раскомментирование в Docker Compose

Откройте `docker-compose.yml`:
- Раскомментируйте секцию вашего сервиса.
- Не забудьте также раскомментировать строку с этим сервисом в секции `nginx:depends_on:`.

### Шаг 5. Раскомментирование в Nginx

Откройте `nginx/nginx.conf` и раскомментируйте конфигурацию для вашего сервиса в блоках `http` и `listen`.

### Шаг 6. Пересборка Docker‑образов

```bash
sudo docker compose build
```

После выполнения этих шагов сервис будет готов к запуску в составе общего стека.

**Важно:** не забудьте изменить порт в `config_vars.yaml`, чтобы сервис запускался на нужном порту.

---

## Частые проблемы и их решение

### Ошибка инициализации StackUsageMonitor

```
CRITICAL <userver> ERROR at userver/core/src/engine/coro/stack_usage_monitor.cpp:226:LogWarningWithErrno. Assertion 'false' failed: Failed to initialize StackUsageMonitor(userfaultfd), errno: 38 (Function not implemented)
```

**Решение:** Убедитесь, что в `static_config.yaml` вашего сервиса добавлена настройка:

```yaml
coro_pool:
    stack_usage_monitor_enabled: false
```

### Сервис не отвечает

- Проверьте, что сервис раскомментирован в `docker-compose.yml` и `nginx/nginx.conf`.
- Убедитесь, что сервис запущен: `sudo docker compose ps`.
- Посмотрите логи Nginx: `sudo docker compose logs nginx`.

---

## Полезные ссылки

- [Документация по сборке userver](https://userver.tech/de/dab/md_en_2userver_2build_2build.html)
- [Туториал «Hello World» на userver](https://userver.tech/da/d16/md_en_2userver_2tutorial_2hello__service.html)
- [Официальный сайт userver](https://userver.tech)
