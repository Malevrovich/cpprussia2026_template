# Start via `make test-debug` or `make test-release`


async def test_ping(service_client):
    """Test that the service is up and responding to ping."""
    response = await service_client.get("/ping")
    assert response.status == 200
    assert response.text == ""


async def test_health_check(service_client):
    """Test basic health check via ping endpoint."""
    response = await service_client.get("/ping")
    assert response.status == 200
