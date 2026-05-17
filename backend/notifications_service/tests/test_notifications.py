"""Tests for notifications service endpoints."""

import json
import uuid
import sys
import os
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '../../..'))
from backend.common.test_utils import generate_token


async def test_create_notification(service_client):
    """Test creating a new notification."""
    request_data = {
        "current_user": {
            "token": generate_token(),
            "login": "alice",
            "name": "Alice Smith"
        },
        "channel_id": 123,
        "message_id": 456,
        "other_user_login": "bob"
    }
    
    response = await service_client.post(
        "/v1/channel/notification/new",
        data=json.dumps(request_data),
        headers={"Content-Type": "application/json"}
    )
    
    assert response.status == 200
    data = response.json()
    assert "notification_id" in data
    # Check it's a valid UUID format
    try:
        uuid.UUID(data["notification_id"])
    except ValueError:
        assert False, f"Invalid UUID: {data['notification_id']}"


async def test_list_notifications_empty(service_client):
    """Test listing notifications when none exist for this user/channel."""
    # Use a different user and channel that haven't been used before
    request_data = {
        "current_user": {
            "token": generate_token(),
            "login": "user_with_no_notifications",
            "name": "No Notifications User"
        },
        "channel_id": 99999  # Channel that doesn't have notifications
    }
    
    response = await service_client.post(
        "/v1/channel/notification/list",
        data=json.dumps(request_data),
        headers={"Content-Type": "application/json"}
    )
    
    assert response.status == 200
    data = response.json()
    assert "notifications" in data
    assert data["notifications"] == []


async def test_create_and_list_notifications(service_client):
    """Test creating a notification and then listing it."""
    # Create a notification
    create_request = {
        "current_user": {
            "token": generate_token(),
            "login": "charlie",
            "name": "Charlie Brown"
        },
        "channel_id": 789,
        "message_id": 999,
        "other_user_login": "david"
    }
    
    create_response = await service_client.post(
        "/v1/channel/notification/new",
        data=json.dumps(create_request),
        headers={"Content-Type": "application/json"}
    )
    
    assert create_response.status == 200
    notification_id = create_response.json()["notification_id"]
    
    # List notifications for the target user
    list_request = {
        "current_user": {
            "token": generate_token(),
            "login": "david",
            "name": "David Wilson"
        },
        "channel_id": 789
    }
    
    list_response = await service_client.post(
        "/v1/channel/notification/list",
        data=json.dumps(list_request),
        headers={"Content-Type": "application/json"}
    )
    
    assert list_response.status == 200
    list_data = list_response.json()
    
    assert len(list_data["notifications"]) == 1
    notification = list_data["notifications"][0]
    assert notification["message_id"] == 999
    assert notification["read"] is False
    
    # List notifications for a different user (should be empty)
    list_request2 = {
        "current_user": {
            "token": generate_token(),
            "login": "charlie",
            "name": "Charlie Brown"
        },
        "channel_id": 789
    }
    
    list_response2 = await service_client.post(
        "/v1/channel/notification/list",
        data=json.dumps(list_request2),
        headers={"Content-Type": "application/json"}
    )
    
    assert list_response2.status == 200
    list_data2 = list_response2.json()
    assert list_data2["notifications"] == []


async def test_create_notification_missing_fields(service_client):
    """Test creating a notification with missing required fields."""
    request_data = {
        "current_user": {
            "token": generate_token(),
            "login": "alice",
            "name": "Alice Smith"
        },
        "channel_id": 123
        # Missing message_id and other_user_login
    }
    
    response = await service_client.post(
        "/v1/channel/notification/new",
        data=json.dumps(request_data),
        headers={"Content-Type": "application/json"}
    )
    
    # Should return 400 Bad Request
    assert response.status == 400


async def test_list_notifications_different_channels(service_client):
    """Test that notifications are scoped to specific channels."""
    # Create notification in channel 100
    create_request1 = {
        "current_user": {
            "token": generate_token(),
            "login": "user1",
            "name": "User One"
        },
        "channel_id": 100,
        "message_id": 1,
        "other_user_login": "user2"
    }
    
    await service_client.post(
        "/v1/channel/notification/new",
        data=json.dumps(create_request1),
        headers={"Content-Type": "application/json"}
    )
    
    # Create notification in channel 200
    create_request2 = {
        "current_user": {
            "token": generate_token(),
            "login": "user1",
            "name": "User One"
        },
        "channel_id": 200,
        "message_id": 2,
        "other_user_login": "user2"
    }
    
    await service_client.post(
        "/v1/channel/notification/new",
        data=json.dumps(create_request2),
        headers={"Content-Type": "application/json"}
    )
    
    # List notifications in channel 100
    list_request1 = {
        "current_user": {
            "token": generate_token(),
            "login": "user2",
            "name": "User Two"
        },
        "channel_id": 100
    }
    
    list_response1 = await service_client.post(
        "/v1/channel/notification/list",
        data=json.dumps(list_request1),
        headers={"Content-Type": "application/json"}
    )
    
    assert list_response1.status == 200
    list_data1 = list_response1.json()
    assert len(list_data1["notifications"]) == 1
    assert list_data1["notifications"][0]["message_id"] == 1
    
    # List notifications in channel 200
    list_request2 = {
        "current_user": {
            "token": generate_token(),
            "login": "user2",
            "name": "User Two"
        },
        "channel_id": 200
    }
    
    list_response2 = await service_client.post(
        "/v1/channel/notification/list",
        data=json.dumps(list_request2),
        headers={"Content-Type": "application/json"}
    )
    
    assert list_response2.status == 200
    list_data2 = list_response2.json()
    assert len(list_data2["notifications"]) == 1
    assert list_data2["notifications"][0]["message_id"] == 2