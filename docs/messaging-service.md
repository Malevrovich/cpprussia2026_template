# Messaging Service

Сервис работы с каналами и сообщениями.

## Функциональность

- **Отправка сообщений** — создание нового сообщения в канале
- **Получение сообщений** — получение списка сообщений по timestamp-диапазону

## API

| Метод | Endpoint | Описание |
|-------|----------|----------|
| POST | /v1/channel/message/new | Отправить новое сообщение |
| POST | /v1/channel/message/by-timestamp | Получить сообщения за период |

## Схема данных

- **V1ChannelId** — ID канала (int64, предопределённые каналы)
- **V1MessageId** — уникальный ID сообщения в канале (int64)
- **V1CurrentUser** — информация о текущем пользователе (token, login, name)
- **V1ChannelMessage** — структура сообщения (id, timestamp, message, current_user)
- **V1Error** — структура ошибки (error, code)

### Пагинация

`V1ChannelMessageByTimestampResponse` содержит:
- `messages` — массив сообщений
- `next_cursor` — курсор для следующей страницы (null если нет больше результатов)
- `has_more` — флаг наличия дополнительных сообщений

### Формат timestamp

Все поля timestamp используют формат ISO8601 (`format: date-time`).

### HTTP Status Codes

| Код | Описание |
|-----|----------|
| 200 | Успешный ответ |
| 400 | Bad Request |
| 401 | Unauthorized |
| 404 | Channel Not Found |
| 500 | Internal Server Error |

## Потенциальные проблемы

- Нет возможности создать канал (описание says "all IDs already exist")
- Нет редактирования/удаления сообщений
