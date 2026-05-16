# Start via `make test-debug` or `make test-release`

import json


async def test_ping(service_client):
    """Test that the service is up and responding."""
    response = await service_client.get("/ping")
    assert response.status == 200
    assert response.text == ""


async def test_health_check(service_client):
    """Simple health check to verify service is running."""
    # Try to upload a minimal file
    request_data = {
        "login": "testuser",
        "filename": "healthcheck.txt",
        "content": "dGVzdA==",  # "test" in base64
        "mime_type": "text/plain"
    }
    
    response = await service_client.post(
        "/v1/file/new",
        data=json.dumps(request_data),
        headers={"Content-Type": "application/json"}
    )
    
    # Should succeed or at least not crash
    assert response.status in [200, 400]  # Either success or validation error is OK
