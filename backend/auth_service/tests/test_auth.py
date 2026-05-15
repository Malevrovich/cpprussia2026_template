"""
Functional tests for authentication endpoints.

These tests are written according to the OpenAPI spec (docs/auth-service.yaml)
and may fail if the implementation does not yet comply.
"""
import pytest


async def test_registration_happy_path(service_client):
    """Successful registration returns 200 with user data and token."""
    request = {
        "login": "testuser",
        "name": "Test User",
        "email": "test@example.com",
        "phone": "+1234567890",
        "password": "securePassword123",
    }
    response = await service_client.post("/v1/user/registration", json=request)
    assert response.status == 200
    assert response.headers["Content-Type"] == "application/json"
    body = response.json()
    assert "current_user" in body
    user = body["current_user"]
    assert user["login"] == "testuser"
    assert user["name"] == "Test User"
    assert "token" in user
    token = user["token"]
    assert isinstance(token, str)
    # Token must be exactly 128 characters as per spec
    assert len(token) == 128, f"Token length {len(token)} != 128"


async def test_registration_missing_required_field(service_client):
    """Missing required field results in 400 validation error."""
    request = {
        "login": "testuser",
        "name": "Test User",
        "email": "test@example.com",
        # missing phone
        "password": "securePassword123",
    }
    response = await service_client.post("/v1/user/registration", json=request)
    assert response.status == 400
    assert response.headers["Content-Type"] == "application/json"
    body = response.json()
    assert "code" in body
    assert body["code"] == "validation_error"
    assert "message" in body


async def test_registration_invalid_email_format(service_client):
    """Invalid email format should cause validation error (400)."""
    request = {
        "login": "testuser",
        "name": "Test User",
        "email": "not-an-email",
        "phone": "+1234567890",
        "password": "securePassword123",
    }
    response = await service_client.post("/v1/user/registration", json=request)
    assert response.status == 400
    assert response.headers["Content-Type"] == "application/json"
    body = response.json()
    assert body["code"] == "validation_error"


async def test_registration_short_password(service_client):
    """Password shorter than 6 characters should be rejected (400)."""
    request = {
        "login": "testuser",
        "name": "Test User",
        "email": "test@example.com",
        "phone": "+1234567890",
        "password": "short",
    }
    response = await service_client.post("/v1/user/registration", json=request)
    assert response.status == 400
    assert response.headers["Content-Type"] == "application/json"
    body = response.json()
    assert body["code"] == "validation_error"


async def test_registration_duplicate_user(service_client):
    """Registering the same login twice should result in conflict (409)."""
    request = {
        "login": "duplicate",
        "name": "Duplicate User",
        "email": "dup@example.com",
        "phone": "+1234567890",
        "password": "securePassword123",
    }
    # First registration
    response1 = await service_client.post("/v1/user/registration", json=request)
    assert response1.status == 200
    # Second registration - must be 409 Conflict
    response2 = await service_client.post("/v1/user/registration", json=request)
    assert response2.status == 409
    assert response2.headers["Content-Type"] == "application/json"
    body = response2.json()
    assert body["code"] == "USER_ALREADY_EXISTS"


async def test_authorization_happy_path(service_client):
    """Successful authorization returns 200 with user data and token."""
    # First register a user (prerequisite)
    reg_request = {
        "login": "authuser",
        "name": "Auth User",
        "email": "auth@example.com",
        "phone": "+1234567890",
        "password": "securePassword123",
    }
    reg_response = await service_client.post("/v1/user/registration", json=reg_request)
    assert reg_response.status == 200

    # Now authorize
    auth_request = {
        "login": "authuser",
        "password": "securePassword123",
    }
    response = await service_client.post("/v1/user/authorization", json=auth_request)
    assert response.status == 200
    assert response.headers["Content-Type"] == "application/json"
    body = response.json()
    assert "current_user" in body
    user = body["current_user"]
    assert user["login"] == "authuser"
    assert user["name"] == "Auth User"  # name from registration
    assert "token" in user
    token = user["token"]
    assert isinstance(token, str)
    assert len(token) == 128, f"Token length {len(token)} != 128"


async def test_authorization_invalid_credentials(service_client):
    """Wrong password results in 401 Unauthorized."""
    # Register a user
    reg_request = {
        "login": "wrongpass",
        "name": "Wrong Pass",
        "email": "wrong@example.com",
        "phone": "+1234567890",
        "password": "correctPassword",
    }
    reg_response = await service_client.post("/v1/user/registration", json=reg_request)
    assert reg_response.status == 200

    # Attempt authorization with wrong password
    auth_request = {
        "login": "wrongpass",
        "password": "wrongPassword",
    }
    response = await service_client.post("/v1/user/authorization", json=auth_request)
    assert response.status == 401
    assert response.headers["Content-Type"] == "application/json"
    body = response.json()
    assert body["code"] == "INVALID_CREDENTIALS"


async def test_authorization_nonexistent_user(service_client):
    """Authorization for non-existent user should return 401."""
    auth_request = {
        "login": "nonexistent",
        "password": "anypassword",
    }
    response = await service_client.post("/v1/user/authorization", json=auth_request)
    assert response.status == 401
    assert response.headers["Content-Type"] == "application/json"
    body = response.json()
    assert body["code"] == "INVALID_CREDENTIALS"


async def test_authorization_missing_field(service_client):
    """Missing required field results in 400 validation error."""
    auth_request = {
        "login": "test",
        # missing password
    }
    response = await service_client.post("/v1/user/authorization", json=auth_request)
    assert response.status == 400
    assert response.headers["Content-Type"] == "application/json"
    body = response.json()
    assert body["code"] == "validation_error"


async def test_wrong_method(service_client):
    """GET, PUT, DELETE etc. should return 405 Method Not Allowed."""
    response = await service_client.get("/v1/user/registration")
    assert response.status == 405
    response = await service_client.put("/v1/user/registration")
    assert response.status == 405
    response = await service_client.delete("/v1/user/registration")
    assert response.status == 405

    response = await service_client.get("/v1/user/authorization")
    assert response.status == 405
    response = await service_client.put("/v1/user/authorization")
    assert response.status == 405
    response = await service_client.delete("/v1/user/authorization")
    assert response.status == 405