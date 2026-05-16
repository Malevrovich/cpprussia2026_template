"""Tests for files service endpoints."""

import json
import base64


async def test_upload_file(service_client):
    """Test uploading a new file."""
    # Create a simple text file content
    file_content = "Hello, World! This is a test file."
    encoded_content = base64.b64encode(file_content.encode()).decode()
    
    request_data = {
        "login": "user123",
        "filename": "test.txt",
        "content": encoded_content,
        "mime_type": "text/plain",
        "size": len(file_content)
    }
    
    response = await service_client.post(
        "/v1/file/new",
        data=json.dumps(request_data),
        headers={"Content-Type": "application/json"}
    )
    
    assert response.status == 200
    data = response.json()
    
    # Check response structure
    assert "current_user" in data
    assert data["current_user"]["login"] == "user123"
    assert "uri" in data
    assert data["uri"].startswith("s3://files/")
    assert "file" in data
    assert data["file"]["login"] == "user123"
    assert data["file"]["filename"] == "test.txt"
    assert data["file"]["content"] == encoded_content
    assert data["file"]["mime_type"] == "text/plain"
    assert data["file"]["size"] == len(file_content)
    
    return data["uri"]  # Return URI for use in other tests


async def test_retrieve_file_by_uri(service_client):
    """Test retrieving a file by its URI."""
    # First upload a file
    file_content = "Test content for retrieval"
    encoded_content = base64.b64encode(file_content.encode()).decode()
    
    upload_request = {
        "login": "alice",
        "filename": "retrieve_test.txt",
        "content": encoded_content,
        "mime_type": "text/plain"
    }
    
    upload_response = await service_client.post(
        "/v1/file/new",
        data=json.dumps(upload_request),
        headers={"Content-Type": "application/json"}
    )
    
    assert upload_response.status == 200
    upload_data = upload_response.json()
    file_uri = upload_data["uri"]
    
    # Now retrieve the file
    retrieve_request = {
        "current_user": {
            "token": "",
            "login": "alice",
            "name": "Alice Smith"
        },
        "uri": file_uri
    }
    
    response = await service_client.post(
        "/v1/file/by-uri",
        data=json.dumps(retrieve_request),
        headers={"Content-Type": "application/json"}
    )
    
    assert response.status == 200
    data = response.json()
    
    assert "file" in data
    assert data["file"]["login"] == "alice"
    assert data["file"]["filename"] == "retrieve_test.txt"
    assert data["file"]["content"] == encoded_content


async def test_retrieve_file_wrong_owner(service_client):
    """Test that a user cannot retrieve a file they don't own."""
    # Upload a file as alice
    file_content = "Private content"
    encoded_content = base64.b64encode(file_content.encode()).decode()
    
    upload_request = {
        "login": "alice",
        "filename": "private.txt",
        "content": encoded_content,
        "mime_type": "text/plain"
    }
    
    upload_response = await service_client.post(
        "/v1/file/new",
        data=json.dumps(upload_request),
        headers={"Content-Type": "application/json"}
    )
    
    assert upload_response.status == 200
    upload_data = upload_response.json()
    file_uri = upload_data["uri"]
    
    # Try to retrieve as bob (different user)
    retrieve_request = {
        "current_user": {
            "token": "",
            "login": "bob",  # Different user!
            "name": "Bob Johnson"
        },
        "uri": file_uri
    }
    
    response = await service_client.post(
        "/v1/file/by-uri",
        data=json.dumps(retrieve_request),
        headers={"Content-Type": "application/json"}
    )
    
    # Should get 403 Forbidden
    assert response.status == 403
    data = response.json()
    assert "error" in data
    assert "owner" in data["error"].lower() or "forbidden" in data["error"].lower()


async def test_retrieve_nonexistent_file(service_client):
    """Test retrieving a file that doesn't exist."""
    retrieve_request = {
        "current_user": {
            "token": "",
            "login": "user123",
            "name": "Test User"
        },
        "uri": "s3://files/nonexistent-file"
    }
    
    response = await service_client.post(
        "/v1/file/by-uri",
        data=json.dumps(retrieve_request),
        headers={"Content-Type": "application/json"}
    )
    
    # Should get 404 Not Found
    assert response.status == 404
    data = response.json()
    assert "error" in data
    assert "not found" in data["error"].lower()


async def test_upload_file_missing_required_fields(service_client):
    """Test uploading a file with missing required fields."""
    # Missing filename
    request_data = {
        "login": "user123",
        "content": "dGVzdA=="  # "test" in base64
    }
    
    response = await service_client.post(
        "/v1/file/new",
        data=json.dumps(request_data),
        headers={"Content-Type": "application/json"}
    )
    
    # Should get 400 Bad Request
    assert response.status == 400
    data = response.json()
    assert "error" in data


async def test_upload_file_empty_content(service_client):
    """Test uploading a file with empty content."""
    request_data = {
        "login": "user123",
        "filename": "empty.txt",
        "content": "",  # Empty content
        "mime_type": "text/plain"
    }
    
    response = await service_client.post(
        "/v1/file/new",
        data=json.dumps(request_data),
        headers={"Content-Type": "application/json"}
    )
    
    # Should get 400 Bad Request
    assert response.status == 400
    data = response.json()
    assert "error" in data


async def test_upload_file_compute_size_automatically(service_client):
    """Test that size is computed automatically if not provided."""
    file_content = "Auto size test"
    encoded_content = base64.b64encode(file_content.encode()).decode()
    
    request_data = {
        "login": "user123",
        "filename": "auto_size.txt",
        "content": encoded_content,
        "mime_type": "text/plain"
        # Note: size field is omitted
    }
    
    response = await service_client.post(
        "/v1/file/new",
        data=json.dumps(request_data),
        headers={"Content-Type": "application/json"}
    )
    
    assert response.status == 200
    data = response.json()
    
    # Size should be computed automatically
    assert "file" in data
    assert data["file"]["size"] == len(file_content)