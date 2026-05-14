Перед вами проект мессенджера без реализованного бекенда

Ваша задача реализовать 6 микросервисов

Ваш проект должен быть реализован на userver framework

---

Фронтенд 

У вас есть готовый фронтенд

```
cd frontend
npm install
npm run build
```

Это создаст папку dist, которая будет деплоиться в докере

---

Докер компоуз

У вас есть готовый докер-компоуз

```
sudo docker compose build
sudo docker compose up -d
```

Важно: докер ничего не билдит и не собирает, он ожидает что фронт был собран
и раскоменченные сервисы тоже успешно побилдились

Фронтенд будет находиться на порту 80

---
Создайте 6 сервисов (смотрите в docs):

1. status_service
2. reactions_service
3. comments_service
4. posts_service
5. users_service
6. auth_service

(если решите поменять имя, то не забудьте поменять docker-compose.yml)

Используйте userver-create-service

https://userver.tech/de/dab/md_en_2userver_2build_2build.html
https://userver.tech/da/d16/md_en_2userver_2tutorial_2hello__service.html

---
Может возникать ошибка похожая на:
```
CRITICAL <userver> ERROR at userver/core/src/engine/coro/stack_usage_monitor.cpp:226:LogWarningWithErrno. Assertion 'false' failed: Failed to initialize StackUsageMonitor(userfaultfd), errno: 38 (Function not implemented)
ERROR
```

Для того чтобы починить добавить в static_config.yaml:
```
coro_pool:
    stack_usage_monitor_enabled: false
```

---
