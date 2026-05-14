# Reactions Service

Сервис реакций (лайков/дизлайков) на сообщения.

## Функциональность

- **Toggle реакции** — добавление/удаление анимации на сообщение

## API

| Метод | Endpoint | Описание |
|-------|----------|----------|
| POST | /v1/like/trigger | Добавить/убрать реакцию |
| GET | /v1/like/{channel_id}/{message_id} | Получить все реакции на сообщение |

## Схема данных

### Enums

**V1Animation** — тип анимации:
- `like` — лайк
- `dislike` — дизлайк
- `heart` — сердце
- `fire` — огонь
- `okay` — окей
- `LOL` — смех
- `smile` — улыбка

### Request/Response

**V1LikeTriggerRequest** — запрос на toggle реакции:
- `current_user` — информация о пользователе
- `idempotency_token` — токен для идемпотентности (16-256 символов)
- `channel_id` — ID канала
- `message_id` — ID сообщения
- `animation` — тип анимации

**V1LikeTriggerResponse** — ответ после toggle:
- `action` — результат: `added` | `removed`
- `current_user_reaction` — текущая анимация пользователя на этом сообщении (null если нет)

**V1GetReactionsResponse** — список реакций:
- `reactions` — массив записей с `user` и `animation`

## Idempotency Token

Механизм toggle с idempotency_token:

1. Если токен используется впервые — выполняется toggle (добавление или удаление в зависимости от текущего состояния)
2. Если токен уже использовался для того же user+channel+message+animation:
   - Возвращается текущее состояние **без изменений**
   - Это позволяет безопасно повторять запросы

## HTTP Status Codes

| Код | Описание |
|-----|----------|
| 200 | Успех |
| 400 | Некорректный запрос |
| 401 | Неавторизован |
| 404 | Сообщение не найдено |
| 409 | Конфликт (idempotency token conflict) |

## Потенциальные проблемы

- [x] Добавлен response с состоянием после toggle
- [x] Объяснён механизм idempotency_token
- [x] Заменён "shit" на "fire"
- [x] Добавлен endpoint для получения реакций
