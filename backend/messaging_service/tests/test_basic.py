# Start via `make test-debug` or `make test-release`


async def test_basic(service_client):
    # Test that the service is running by checking ping endpoint
    response = await service_client.get("/ping")
    assert response.status == 200
    
    # Test that our messaging endpoint is available
    response = await service_client.post(
        "/v1/channel/message/new",
        json={
            "current_user": {
                "token": "a1b2c3d4e5f6a1b2c3d4e5f6a1b2c3d4e5f6a1b2c3d4e5f6a1b2c3d4e5f6a1b2c3d4e5f6a1b2c3d4e5f6a1b2c3d4e5f6a1b2c3d4e5f6",
                "login": "testuser",
                "name": "Test User"
            },
            "channel_id": 1,
            "message": "Test message"
        }
    )
    # Should either succeed (200) or return validation error (400) but not 404
    assert response.status in (200, 400)
