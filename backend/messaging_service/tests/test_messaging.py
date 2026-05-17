# Functional tests for messaging service endpoints

import pytest
import sys
import os
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '../../..'))
from backend.common.test_utils import generate_token


async def test_message_new_success(service_client):
    """Test successful creation of a new message."""
    request_data = {
        "current_user": {
            "token": generate_token(),
            "login": "testuser",
            "name": "Test User"
        },
        "channel_id": 1,
        "message": "Hello, this is a test message!"
    }
    
    response = await service_client.post(
        "/v1/channel/message/new",
        json=request_data
    )
    
    assert response.status == 200
    data = response.json()
    assert "message_id" in data
    assert isinstance(data["message_id"], int)
    assert data["message_id"] > 0


async def test_message_new_empty_message(service_client):
    """Test creating a message with empty content."""
    request_data = {
        "current_user": {
            "token": generate_token(),
            "login": "testuser",
            "name": "Test User"
        },
        "channel_id": 1,
        "message": ""
    }
    
    response = await service_client.post(
        "/v1/channel/message/new",
        json=request_data
    )
    
    assert response.status == 400
    data = response.json()
    assert "error" in data
    assert "code" in data
    assert data["code"] == 400


async def test_message_new_missing_fields(service_client):
    """Test creating a message with missing required fields."""
    request_data = {
        "current_user": {
            "login": "testuser",
            "name": "Test User"
        },
        # Missing channel_id and message
    }
    
    response = await service_client.post(
        "/v1/channel/message/new",
        json=request_data
    )
    
    assert response.status == 400
    data = response.json()
    assert "error" in data
    assert "code" in data


async def test_message_by_timestamp_success(service_client):
    """Test successful retrieval of messages by timestamp."""
    # First, create a message to ensure there's data
    create_request = {
        "current_user": {
            "token": generate_token(),
            "login": "testuser",
            "name": "Test User"
        },
        "channel_id": 1,
        "message": "Test message for retrieval"
    }
    
    create_response = await service_client.post(
        "/v1/channel/message/new",
        json=create_request
    )
    assert create_response.status == 200
    
    # Now retrieve messages
    retrieve_request = {
        "channel_id": 1,
        "from": "2026-01-01T00:00:00Z",
        "limit": 10
    }
    
    response = await service_client.post(
        "/v1/channel/message/by-timestamp",
        json=retrieve_request
    )
    
    assert response.status == 200
    data = response.json()
    assert "messages" in data
    assert isinstance(data["messages"], list)
    assert "has_more" in data
    assert "next_cursor" in data
    
    # Should have at least the message we just created
    if len(data["messages"]) > 0:
        message = data["messages"][0]
        assert "id" in message
        assert "timestamp" in message
        assert "message" in message
        assert "current_user" in message
        assert message["current_user"]["login"] == "testuser"


async def test_message_by_timestamp_with_to_date(service_client):
    """Test retrieval with both from and to dates."""
    request_data = {
        "channel_id": 1,
        "from": "2026-01-01T00:00:00Z",
        "to": "2026-12-31T23:59:59Z",
        "limit": 5
    }
    
    response = await service_client.post(
        "/v1/channel/message/by-timestamp",
        json=request_data
    )
    
    assert response.status == 200
    data = response.json()
    assert "messages" in data
    assert isinstance(data["messages"], list)


async def test_message_by_timestamp_invalid_limit(service_client):
    """Test retrieval with invalid limit values."""
    # Test limit too small
    request_data = {
        "channel_id": 1,
        "from": "2026-01-01T00:00:00Z",
        "limit": 0
    }
    
    response = await service_client.post(
        "/v1/channel/message/by-timestamp",
        json=request_data
    )
    
    assert response.status == 400
    data = response.json()
    assert "error" in data
    assert data["code"] == 400
    
    # Test limit too large
    request_data["limit"] = 2000
    
    response = await service_client.post(
        "/v1/channel/message/by-timestamp",
        json=request_data
    )
    
    assert response.status == 400
    data = response.json()
    assert "error" in data
    assert data["code"] == 400


async def test_message_by_timestamp_missing_from(service_client):
    """Test retrieval without required from date."""
    request_data = {
        "channel_id": 1,
        "limit": 10
        # Missing 'from'
    }
    
    response = await service_client.post(
        "/v1/channel/message/by-timestamp",
        json=request_data
    )
    
    assert response.status == 400
    data = response.json()
    assert "error" in data
    assert data["code"] == 400


async def test_multiple_messages_same_channel(service_client):
    """Test creating and retrieving multiple messages in the same channel."""
    channel_id = 2
    
    # Create multiple messages
    for i in range(3):
        request_data = {
            "current_user": {
                "token": generate_token(),
                "login": f"user{i}",
                "name": f"User {i}"
            },
            "channel_id": channel_id,
            "message": f"Test message {i}"
        }
        
        response = await service_client.post(
            "/v1/channel/message/new",
            json=request_data
        )
        assert response.status == 200
    
    # Retrieve all messages
    retrieve_request = {
        "channel_id": channel_id,
        "from": "2026-01-01T00:00:00Z",
        "limit": 10
    }
    
    response = await service_client.post(
        "/v1/channel/message/by-timestamp",
        json=retrieve_request
    )
    
    assert response.status == 200
    data = response.json()
    assert len(data["messages"]) >= 3
    
    # Verify all messages are from the correct channel
    for message in data["messages"]:
        assert message["current_user"]["login"].startswith("user")
        assert message["message"].startswith("Test message")


async def test_different_channels_independent(service_client):
    """Test that messages in different channels are independent."""
    # Create message in channel 3
    request_data_ch3 = {
        "current_user": {
            "token": generate_token(),
            "login": "user_ch3",
            "name": "User Channel 3"
        },
        "channel_id": 3,
        "message": "Message in channel 3"
    }
    
    response = await service_client.post(
        "/v1/channel/message/new",
        json=request_data_ch3
    )
    assert response.status == 200
    
    # Create message in channel 4
    request_data_ch4 = {
        "current_user": {
            "token": generate_token(),
            "login": "user_ch4",
            "name": "User Channel 4"
        },
        "channel_id": 4,
        "message": "Message in channel 4"
    }
    
    response = await service_client.post(
        "/v1/channel/message/new",
        json=request_data_ch4
    )
    assert response.status == 200
    
    # Retrieve messages from channel 3 only
    retrieve_request = {
        "channel_id": 3,
        "from": "2026-01-01T00:00:00Z",
        "limit": 10
    }
    
    response = await service_client.post(
        "/v1/channel/message/by-timestamp",
        json=retrieve_request
    )
    
    assert response.status == 200
    data = response.json()
    
    # Should only have messages from channel 3
    for message in data["messages"]:
        assert message["current_user"]["login"] == "user_ch3"
        assert message["message"] == "Message in channel 3"