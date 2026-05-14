# Status Service

Сервис пользовательских статусов.

## Функциональность

- **Обновление статуса** — установка статуса с типом и сообщением
- **Получение статуса** — получение статуса по login пользователя

## API

| Метод | Endpoint | Описание |
|-------|----------|----------|
| POST | /v1/user/status/update | Обновить свой статус |
| POST | /v1/user/status/by-login | Получить статус пользователя |

## Схема данных

### V1StatusType
Тип статуса пользователя:
- `online` — пользователь онлайн
- `away` — пользователь отсутствует
- `busy` — пользователь занят
- `offline` — пользователь офлайн

### V1Visibility
Видимость статуса:
- `public` — статус виден всем
- `private` — статус виден только владельцу

### V1UserStatus
```json
{
  "status_type": "online",
  "status_message": "Working from home",
  "visibility": "public"
}
```

### V1UserStatusUpdateRequest
- `current_user` — информация о текущем пользователе
- `status` — объект V1UserStatus

### V1UserStatusUpdateResponse
- `success` — успешность операции
- `updated_at` — время обновления статуса
- `expires_at` — время истечения статуса (опционально)

### V1UserStatusByLoginRequest
- `current_user` — информация о текущем пользователе
- `login` — login пользователя, чей статус запрашивается

### V1UserStatusByLoginResponse
- `status` — объект V1UserStatus

## HTTP Status Codes

| Код | Описание |
|-----|----------|
| 200 | Успешный запрос |
| 400 | Bad request — некорректный запрос |
| 401 | Unauthorized — пользователь не аутентифицирован |
| 403 | Forbidden — нет доступа к запрашиваемому ресурсу |
| 404 | Not found — пользователь не найден |
| 500 | Internal server error — внутренняя ошибка сервера |

## Потенциальные проблемы

- Нет TTL или automatic cleanup
- Нет списка всех статусов
