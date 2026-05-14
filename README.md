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
│   ├── CMakeLists.txt         # CMake‑конфигурация всех сервисов
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
- Раскомментировать секции только тех сервисов, которые уже готовы, в `docker-compose.yml`, `backend/CMakeLists.txt` и `nginx/nginx.conf`

Фронтенд будет доступен по адресу **http://localhost:80**.

---

## Список сервисов

Вам предстоит реализовать шесть микросервисов. Каждый сервис описан в отдельном файле в папке `docs/`.

| № | Сервис | Назначение | Документация |
|---|--------|------------|--------------|
| 1 | **status_service** | Отслеживание онлайн‑статуса пользователей | [`docs/status-service.md`](docs/status-service.md) |
| 2 | **reactions_service** | Реакции на сообщения (лайки, эмодзи) | [`docs/reactions-service.md`](docs/reactions-service.md) |
| 3 | **comments_service** | Комментарии к сообщениям и постам | [`docs/comments-service.md`](docs/comments-service.md) |
| 4 | **posts_service** | Публикации и треды сообщений | [`docs/posts-service.md`](docs/posts-service.md) |
| 5 | **users_service** | Управление профилями пользователей | [`docs/users-service.md`](docs/users-service.md) |
| 6 | **auth_service** | Аутентификация и авторизация | [`docs/auth-service.md`](docs/auth-service.md) |

**Примечание:** Если вы решите изменить имя какого‑либо сервиса, не забудьте обновить его название в `docker-compose.yml`, `backend/CMakeLists.txt` и `nginx/nginx.conf`.

---

## Сборка микросервисов

Перед запуском сервисов необходимо собрать их бинарные файлы. Docker Compose ожидает, что папка `build/` будет находиться в корне проекта (рядом с `docker-compose.yml`).

Выполните из корневой директории проекта:

```bash
cmake -B build -S backend
cmake --build build -j$(nproc)
```

Эта команда создаст папку `build/` в корне и скомпилирует все сервисы, которые раскомментированы в `backend/CMakeLists.txt`.

После успешной сборки бинарники будут автоматически скопированы в соответствующие места для Docker‑образа.

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

### Шаг 3. Раскомментирование в CMake

Откройте `backend/CMakeLists.txt` и раскомментируйте блок, соответствующий вашему сервису.

### Шаг 4. Раскомментирование в Docker Compose

Откройте `docker-compose.yml`:
- Раскомментируйте секцию вашего сервиса.
- Не забудьте также раскомментировать строку с этим сервисом в секции `nginx:depends_on:`.

### Шаг 5. Раскомментирование в Nginx

Откройте `nginx/nginx.conf` и раскомментируйте конфигурацию для вашего сервиса в блоках `http` и `listen`.

### Шаг 6. Пересборка проекта

Выполните сборку из корня проекта (как описано в разделе «Сборка микросервисов»):

```bash
cmake -B build -S backend
cmake --build build -j$(nproc)
```

### Шаг 7. Пересборка Docker‑образов

```bash
sudo docker compose build
```

После выполнения этих шагов сервис будет готов к запуску в составе общего стека.

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

### Ошибки сборки

- Убедитесь, что установлены все зависимости (CMake, компилятор C++20, userver).
- Используйте параллельную сборку: `cmake --build build -j$(nproc)`.

---

## Полезные ссылки

- [Документация по сборке userver](https://userver.tech/de/dab/md_en_2userver_2build_2build.html)
- [Туториал «Hello World» на userver](https://userver.tech/da/d16/md_en_2userver_2tutorial_2hello__service.html)
- [Официальный сайт userver](https://userver.tech)
