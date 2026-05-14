# Files Service

Сервис передачи файлов.

## Функциональность

- **Загрузка файла** — сохранение файла с получением URI
- **Получение файла** — загрузка по ранее полученному URI (доступно только владельцу)

## API

| Метод | Endpoint | Описание |
|-------|----------|----------|
| POST | /v1/file/new | Загрузить новый файл |
| POST | /v1/file/by-uri | Получить файл по URI |

## Схема данных

### V1File
| Поле | Тип | Описание |
|------|-----|----------|
| login | string | Идентификатор владельца файла |
| filename | string | Имя файла |
| content | string (base64) | Содержимое файла в Base64 |
| mime_type | string | MIME тип файла (например, `application/pdf`) |
| size | integer | Размер файла в байтах |

### V1FileNewResponse
| Поле | Тип | Описание |
|------|-----|----------|
| current_user | V1CurrentUser | Информация о текущем пользователе |
| uri | string | URI для доступа к файлу (формат: `s3://bucket/path`) |
| file | V1File | Полная информация о загруженном файле |

### V1FileByUriRequest
| Поле | Тип | Описание |
|------|-----|----------|
| current_user | V1CurrentUser | Текущий пользователь (для проверки авторизации) |
| uri | string | URI файла (`s3://bucket/path`) |

### V1FileByUriResponse
| Поле | Тип | Описание |
|------|-----|----------|
| file | V1File | Полная информация о файле |

## Авторизация

При запросе файла по URI (`POST /v1/file/by-uri`) выполняется проверка:
- `current_user.login` должен совпадать с `file.login` (владелец файла)
- При несовпадении возвращается `403 Forbidden`

## HTTP Status Codes

| Код | Описание |
|-----|----------|
| 200 | Успешный запрос |
| 400 | Bad request |
| 401 | Unauthorized |
| 403 | Forbidden (пользователь не является владельцем файла) |
| 404 | File not found |

## URI Format

URI имеет формат `s3://bucket/path`, где:
- `bucket` — имя хранилища
- `path` — путь к файлу

Пример: `s3://my-bucket/abc123/document.pdf`
