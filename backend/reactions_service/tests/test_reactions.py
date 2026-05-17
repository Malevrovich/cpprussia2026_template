# Functional tests for reactions service
# Start via `make test-debug` or `make test-release`

import json
import uuid
import sys
import os
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '../../..'))
from backend.common.test_utils import generate_token


async def test_like_trigger_add(service_client):
    """Test adding a like reaction."""
    request = {
        "current_user": {
            "token": generate_token(),
            "login": "testuser",
            "name": "Test User"
        },
        "idempotency_token": str(uuid.uuid4()),
        "channel_id": 1,
        "message_id": 100,
        "animation": "like"
    }
    
    response = await service_client.post("/v1/like/trigger", json=request)
    assert response.status == 200
    
    data = response.json()
    assert data["action"] == "added"
    assert data["current_user_reaction"] == "like"


async def test_like_trigger_remove(service_client):
    """Test removing a like reaction (toggle)."""
    # First, add a reaction
    request = {
        "current_user": {
            "token": generate_token(),
            "login": "testuser2",
            "name": "Test User 2"
        },
        "idempotency_token": str(uuid.uuid4()),
        "channel_id": 1,
        "message_id": 101,
        "animation": "heart"
    }
    
    response = await service_client.post("/v1/like/trigger", json=request)
    assert response.status == 200
    assert response.json()["action"] == "added"
    
    # Now toggle (remove) with same parameters
    request["idempotency_token"] = str(uuid.uuid4())
    response = await service_client.post("/v1/like/trigger", json=request)
    assert response.status == 200
    
    data = response.json()
    assert data["action"] == "removed"
    assert data["current_user_reaction"] is None


async def test_like_trigger_idempotency(service_client):
    """Test idempotency token behavior."""
    token = str(uuid.uuid4())
    request = {
        "current_user": {
            "token": generate_token(),
            "login": "testuser3",
            "name": "Test User 3"
        },
        "idempotency_token": token,
        "channel_id": 2,
        "message_id": 200,
        "animation": "fire"
    }
    
    # First request
    response1 = await service_client.post("/v1/like/trigger", json=request)
    assert response1.status == 200
    data1 = response1.json()
    
    # Second request with same token
    response2 = await service_client.post("/v1/like/trigger", json=request)
    assert response2.status == 200
    data2 = response2.json()
    
    # Should return same result without changes
    assert data1 == data2


async def test_like_trigger_idempotency_conflict(service_client):
    """Test idempotency token conflict with different parameters."""
    token = str(uuid.uuid4())
    request1 = {
        "current_user": {
            "token": generate_token(),
            "login": "testuser4",
            "name": "Test User 4"
        },
        "idempotency_token": token,
        "channel_id": 3,
        "message_id": 300,
        "animation": "okay"
    }
    
    # First request
    response1 = await service_client.post("/v1/like/trigger", json=request1)
    assert response1.status == 200
    
    # Second request with same token but different animation (conflict)
    request2 = request1.copy()
    request2["animation"] = "smile"
    
    response2 = await service_client.post("/v1/like/trigger", json=request2)
    assert response2.status == 409  # Conflict


async def test_get_reactions_empty(service_client):
    """Test getting reactions from a message with no reactions."""
    response = await service_client.get("/v1/like/999/9999")
    assert response.status == 200
    
    data = response.json()
    assert data["reactions"] == []


async def test_get_reactions_with_data(service_client):
    """Test getting reactions after adding some."""
    channel_id = 5
    message_id = 500
    
    # Add multiple reactions
    users = ["user1", "user2", "user3"]
    animations = ["like", "dislike", "heart"]
    
    for i, (user, animation) in enumerate(zip(users, animations)):
        request = {
            "current_user": {
                "token": generate_token(),
                "login": user,
                "name": f"User {i+1}"
            },
            "idempotency_token": str(uuid.uuid4()),
            "channel_id": channel_id,
            "message_id": message_id,
            "animation": animation
        }
        response = await service_client.post("/v1/like/trigger", json=request)
        assert response.status == 200
    
    # Get reactions
    response = await service_client.get(f"/v1/like/{channel_id}/{message_id}")
    assert response.status == 200
    
    data = response.json()
    assert len(data["reactions"]) == 3
    
    # Check that all reactions are present
    user_animations = {(r["user"], r["animation"]) for r in data["reactions"]}
    expected = {("user1", "like"), ("user2", "dislike"), ("user3", "heart")}
    assert user_animations == expected


async def test_like_trigger_validation(service_client):
    """Test request validation."""
    # Test with short token
    request = {
        "current_user": {
            "token": "short",
            "login": "test",
            "name": "Test"
        },
        "idempotency_token": str(uuid.uuid4()),
        "channel_id": 1,
        "message_id": 1,
        "animation": "like"
    }
    
    response = await service_client.post("/v1/like/trigger", json=request)
    assert response.status == 401  # JWT validation returns 401 for invalid token
    
    # Test with short idempotency token
    request["current_user"]["token"] = generate_token()
    request["idempotency_token"] = "short"
    response = await service_client.post("/v1/like/trigger", json=request)
    assert response.status == 400
    
    # Test with invalid animation
    request["idempotency_token"] = str(uuid.uuid4())
    request["animation"] = "invalid_animation"
    response = await service_client.post("/v1/like/trigger", json=request)
    assert response.status == 400


async def test_like_trigger_replace_animation(service_client):
    """Test replacing one animation with another."""
    user = "replace_user"
    channel_id = 6
    message_id = 600
    
    # Add like reaction
    request = {
        "current_user": {
            "token": generate_token(),
            "login": user,
            "name": "Replace User"
        },
        "idempotency_token": str(uuid.uuid4()),
        "channel_id": channel_id,
        "message_id": message_id,
        "animation": "like"
    }
    
    response = await service_client.post("/v1/like/trigger", json=request)
    assert response.status == 200
    assert response.json()["action"] == "added"
    assert response.json()["current_user_reaction"] == "like"
    
    # Replace with heart
    request["idempotency_token"] = str(uuid.uuid4())
    request["animation"] = "heart"
    response = await service_client.post("/v1/like/trigger", json=request)
    assert response.status == 200
    assert response.json()["action"] == "added"
    assert response.json()["current_user_reaction"] == "heart"
    
    # Verify only heart reaction exists
    response = await service_client.get(f"/v1/like/{channel_id}/{message_id}")
    assert response.status == 200
    data = response.json()
    assert len(data["reactions"]) == 1
    assert data["reactions"][0]["user"] == user
    assert data["reactions"][0]["animation"] == "heart"