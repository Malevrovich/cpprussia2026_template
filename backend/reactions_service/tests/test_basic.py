# Basic smoke tests for reactions service
# Start via `make test-debug` or `make test-release`

import json
import uuid
import sys
import os
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '../../..'))
from backend.common.test_utils import generate_token


async def test_ping(service_client):
    """Test that the service is alive."""
    response = await service_client.get("/ping")
    assert response.status == 200


async def test_like_trigger_smoke(service_client):
    """Basic smoke test for like trigger endpoint."""
    request = {
        "current_user": {
            "token": generate_token(),
            "login": "smoketest",
            "name": "Smoke Test User"
        },
        "idempotency_token": str(uuid.uuid4()),
        "channel_id": 1,
        "message_id": 1,
        "animation": "like"
    }
    
    response = await service_client.post("/v1/like/trigger", json=request)
    assert response.status == 200
    
    data = response.json()
    assert "action" in data
    assert "current_user_reaction" in data
    assert data["action"] in ["added", "removed"]


async def test_get_reactions_smoke(service_client):
    """Basic smoke test for get reactions endpoint."""
    response = await service_client.get("/v1/like/1/1")
    assert response.status == 200
    
    data = response.json()
    assert "reactions" in data
    assert isinstance(data["reactions"], list)
