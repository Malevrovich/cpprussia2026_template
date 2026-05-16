# Start via `make test-debug` or `make test-release`

import json
import pytest


async def test_status_update_success(service_client):
    """Test successful status update"""
    request = {
        "current_user": {
            "token": "a" * 128,  # 128 characters as per spec
            "login": "john_doe",
            "name": "John Doe"
        },
        "status": {
            "status_type": "online",
            "status_message": "Working from home",
            "visibility": "public"
        }
    }
    
    response = await service_client.post("/v1/user/status/update", json=request)
    assert response.status == 200
    
    data = response.json()
    assert data["success"] is True
    assert "updated_at" in data
    assert "expires_at" in data


async def test_status_update_missing_token(service_client):
    """Test status update with missing token"""
    request = {
        "current_user": {
            "token": "",  # Empty token
            "login": "john_doe",
            "name": "John Doe"
        },
        "status": {
            "status_type": "online",
            "status_message": "Working from home"
        }
    }
    
    response = await service_client.post("/v1/user/status/update", json=request)
    assert response.status == 401
    
    data = response.json()
    assert data["error"] == "unauthorized"
    assert "Token is required" in data["message"]


async def test_status_update_invalid_token_length(service_client):
    """Test status update with invalid token length"""
    request = {
        "current_user": {
            "token": "short",  # Too short
            "login": "john_doe",
            "name": "John Doe"
        },
        "status": {
            "status_type": "online",
            "status_message": "Working from home"
        }
    }
    
    response = await service_client.post("/v1/user/status/update", json=request)
    assert response.status == 401
    
    data = response.json()
    assert data["error"] == "unauthorized"
    assert "Invalid token format" in data["message"]


async def test_status_update_invalid_status_type(service_client):
    """Test status update with invalid status type"""
    request = {
        "current_user": {
            "token": "a" * 128,
            "login": "john_doe",
            "name": "John Doe"
        },
        "status": {
            "status_type": "invalid_type",  # Invalid
            "status_message": "Working from home"
        }
    }
    
    response = await service_client.post("/v1/user/status/update", json=request)
    assert response.status == 400
    
    data = response.json()
    assert data["error"] == "invalid_request"


async def test_status_by_login_success_public(service_client):
    """Test successful retrieval of public status"""
    # First, set a status
    update_request = {
        "current_user": {
            "token": "a" * 128,
            "login": "jane_doe",
            "name": "Jane Doe"
        },
        "status": {
            "status_type": "away",
            "status_message": "In a meeting",
            "visibility": "public"
        }
    }
    
    update_response = await service_client.post("/v1/user/status/update", json=update_request)
    assert update_response.status == 200
    
    # Now retrieve it
    get_request = {
        "current_user": {
            "token": "b" * 128,  # Different user
            "login": "john_doe",
            "name": "John Doe"
        },
        "login": "jane_doe"
    }
    
    response = await service_client.post("/v1/user/status/by-login", json=get_request)
    assert response.status == 200
    
    data = response.json()
    assert data["status"]["status_type"] == "away"
    assert data["status"]["status_message"] == "In a meeting"
    assert data["status"]["visibility"] == "public"
    assert "updated_at" in data


async def test_status_by_login_private_forbidden(service_client):
    """Test retrieval of private status by non-owner"""
    # First, set a private status
    update_request = {
        "current_user": {
            "token": "a" * 128,
            "login": "alice",
            "name": "Alice"
        },
        "status": {
            "status_type": "busy",
            "status_message": "Do not disturb",
            "visibility": "private"
        }
    }
    
    update_response = await service_client.post("/v1/user/status/update", json=update_request)
    assert update_response.status == 200
    
    # Try to retrieve it as a different user
    get_request = {
        "current_user": {
            "token": "b" * 128,
            "login": "bob",
            "name": "Bob"
        },
        "login": "alice"
    }
    
    response = await service_client.post("/v1/user/status/by-login", json=get_request)
    assert response.status == 403
    
    data = response.json()
    assert data["error"] == "forbidden"
    assert "permission" in data["message"].lower()


async def test_status_by_login_private_allowed_for_owner(service_client):
    """Test retrieval of private status by owner"""
    # First, set a private status
    update_request = {
        "current_user": {
            "token": "a" * 128,
            "login": "charlie",
            "name": "Charlie"
        },
        "status": {
            "status_type": "offline",
            "status_message": "Gone fishing",
            "visibility": "private"
        }
    }
    
    update_response = await service_client.post("/v1/user/status/update", json=update_request)
    assert update_response.status == 200
    
    # Retrieve it as the owner
    get_request = {
        "current_user": {
            "token": "a" * 128,  # Same user
            "login": "charlie",
            "name": "Charlie"
        },
        "login": "charlie"
    }
    
    response = await service_client.post("/v1/user/status/by-login", json=get_request)
    assert response.status == 200
    
    data = response.json()
    assert data["status"]["status_type"] == "offline"
    assert data["status"]["visibility"] == "private"


async def test_status_by_login_user_not_found(service_client):
    """Test retrieval for non-existent user"""
    request = {
        "current_user": {
            "token": "a" * 128,
            "login": "john_doe",
            "name": "John Doe"
        },
        "login": "non_existent_user"
    }
    
    response = await service_client.post("/v1/user/status/by-login", json=request)
    assert response.status == 404
    
    data = response.json()
    assert data["error"] == "not_found"
    assert "not found" in data["message"].lower()


async def test_status_update_all_status_types(service_client):
    """Test updating with all possible status types"""
    status_types = ["online", "away", "busy", "offline"]
    
    for i, status_type in enumerate(status_types):
        request = {
            "current_user": {
                "token": "a" * 128,
                "login": f"user_{i}",
                "name": f"User {i}"
            },
            "status": {
                "status_type": status_type,
                "status_message": f"Test message for {status_type}"
            }
        }
        
        response = await service_client.post("/v1/user/status/update", json=request)
        assert response.status == 200
        
        data = response.json()
        assert data["success"] is True


async def test_status_update_all_visibility_types(service_client):
    """Test updating with all possible visibility types"""
    visibilities = ["public", "private"]
    
    for i, visibility in enumerate(visibilities):
        request = {
            "current_user": {
                "token": "a" * 128,
                "login": f"vis_user_{i}",
                "name": f"Visibility User {i}"
            },
            "status": {
                "status_type": "online",
                "status_message": f"Test with {visibility} visibility",
                "visibility": visibility
            }
        }
        
        response = await service_client.post("/v1/user/status/update", json=request)
        assert response.status == 200
        
        data = response.json()
        assert data["success"] is True
