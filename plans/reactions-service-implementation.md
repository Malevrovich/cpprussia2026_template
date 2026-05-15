# Reactions Service Implementation Plan

## Overview
Implement a reactions service with two endpoints as specified in `docs/reactions-service.yaml`:
1. POST `/v1/like/trigger` - Toggle reactions on messages
2. GET `/v1/like/{channel_id}/{message_id}` - Get all reactions on a message

All data will be stored in memory within the service (no external database).

## Architecture

```mermaid
graph TB
    subgraph "Reactions Service"
        Main[main.cpp] -->|registers| ComponentList[Component List]
        ComponentList -->|includes| TriggerHandler[LikeTriggerHandler]
        ComponentList -->|includes| GetHandler[GetReactionsHandler]
        ComponentList -->|includes| Storage[ReactionsStorageComponent]
        
        TriggerHandler -->|depends on| Storage
        GetHandler -->|depends on| Storage
        
        Storage -->|stores| InMemoryData[In-memory Data Structures]
    end
    
    subgraph "External"
        Client[HTTP Client] -->|POST /v1/like/trigger| TriggerHandler
        Client -->|GET /v1/like/{channel}/{message}| GetHandler
    end
```

## Data Structures

### Schemas (schemas.hpp)
Based on OpenAPI specification:
- `V1ChannelId` - `int64_t`
- `V1MessageId` - `int64_t`
- `V1Login` - `std::string` (min 3 chars)
- `V1CurrentUser` - struct with token, login, name
- `V1Animation` - enum class: like, dislike, heart, fire, okay, LOL, smile
- `V1LikeTriggerRequest` - struct with current_user, idempotency_token, channel_id, message_id, animation
- `V1LikeTriggerResponse` - struct with action (added/removed), current_user_reaction (nullable)
- `V1ReactionEntry` - struct with user, animation
- `V1GetReactionsResponse` - struct with reactions vector
- `V1Error` - struct with code, message

### Storage Design
- Thread-safe in-memory storage using `engine::mutex`
- Data structure: `std::map<ChannelId, std::map<MessageId, std::map<UserLogin, Animation>>>`
- Idempotency token tracking: `std::map<std::string, std::tuple<UserLogin, ChannelId, MessageId, Animation, Action>>`

## Components

### 1. ReactionsStorageComponent
- Name: `reactions-storage`
- Inherits from `components::ComponentBase`
- Thread-safe operations with mutex
- Methods:
  - `ToggleReaction()` - handles idempotent toggle logic
  - `GetReactions()` - returns all reactions for a message
  - `MessageExists()` - checks if message exists (all messages exist by spec)

### 2. LikeTriggerHandler
- Name: `handler-like-trigger`
- Path: `/v1/like/trigger`
- Method: POST
- Validates request, calls storage, returns response
- Implements idempotency token logic

### 3. GetReactionsHandler  
- Name: `handler-get-reactions`
- Path: `/v1/like/{channel_id}/{message_id}`
- Method: GET
- Returns all reactions for specified message

## Implementation Steps

### Phase 1: Foundation
1. Create `schemas.hpp` with all data structures
2. Create `reactions_storage_component.hpp/.cpp`
3. Create `like_trigger_handler.hpp/.cpp`
4. Create `get_reactions_handler.hpp/.cpp`

### Phase 2: Integration
5. Update `CMakeLists.txt` to include new source files
6. Update `static_config.yaml` with handler configurations
7. Update `main.cpp` to register new components
8. Remove greeting/hello files and handlers

### Phase 3: Testing
9. Create basic functional tests in `tests/test_reactions.py`
10. Verify service builds and runs correctly

## Idempotency Implementation
The idempotency token mechanism:
1. Store mapping: `token -> (user, channel, message, animation, action)`
2. On request with existing token: return current state without changes
3. On request with new token: perform toggle, store mapping
4. Token reuse for same parameters returns current state

## Error Handling
- 400: Invalid request parameters
- 401: Unauthorized (missing/invalid current_user)
- 404: Message not found (though spec says all messages exist)
- 409: Idempotency token conflict
- 500: Internal server error

## File Changes

### New Files:
- `src/schemas.hpp`
- `src/reactions_storage_component.hpp`
- `src/reactions_storage_component.cpp`
- `src/like_trigger_handler.hpp`
- `src/like_trigger_handler.cpp`
- `src/get_reactions_handler.hpp`
- `src/get_reactions_handler.cpp`

### Modified Files:
- `CMakeLists.txt` - add new source files
- `configs/static_config.yaml` - add handler configurations
- `src/main.cpp` - register new components, remove Hello

### Removed Files:
- `src/greeting.hpp`
- `src/greeting.cpp`
- `src/greeting_test.cpp`
- `src/greeting_benchmark.cpp`
- `src/hello.hpp`
- `src/hello.cpp`

## Dependencies
- Follows same pattern as `messaging_service`
- Uses `userver::components::ComponentBase`
- Uses `userver::engine::mutex` for thread safety
- Uses `userver::server::handlers::HttpHandlerBase`

## Testing Strategy
1. Functional tests using `pytest_userver`
2. Test toggle behavior (add/remove)
3. Test idempotency token logic
4. Test concurrent access
5. Test error cases

## Success Criteria
- Service compiles without errors
- Both endpoints respond correctly
- Idempotency token logic works as specified
- Thread-safe operations under concurrent load
- All greeting/hello functionality removed