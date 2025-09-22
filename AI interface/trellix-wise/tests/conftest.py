"""
Shared test configuration for Trellix WISE API tests.

This module provides common test fixtures, sample data, and configuration
that can be used across all test files.
"""

import pytest
import asyncio
from typing import Dict, Any

# Sample alert data that can be used across all tests
SAMPLE_ALERT_DATA = {
    "_id": 1420,
    "agent": {
        "_id": "test-agent-123",
        "containment_state": "normal"
    },
    "event_values": {
        "detections": {
            "detection": [{
                "infection": {
                    "infection-name": "BD.TestSignature",
                    "confidence-level": "high"
                },
                "infected-object": {
                    "file-object": {
                        "file-path": "/tmp/suspicious.tar.gz",
                        "md5sum": "54d9169d56f98d1f35b38a1c29244803"
                    }
                }
            }]
        },
        "os-details": {
            "name": "Linux",
            "version": "5.15.0-40-generic"
        }
    }
}

# Alternative sample data for testing different scenarios
SAMPLE_ALERT_DATA_WINDOWS = {
    "_id": 1421,
    "agent": {
        "_id": "test-agent-456",
        "containment_state": "normal"
    },
    "event_values": {
        "detections": {
            "detection": [{
                "infection": {
                    "infection-name": "Trojan.GenericKD.12345",
                    "confidence-level": "medium"
                },
                "infected-object": {
                    "file-object": {
                        "file-path": "C:\\Users\\Public\\malware.exe",
                        "md5sum": "d41d8cd98f00b204e9800998ecf8427e"
                    }
                }
            }]
        },
        "os-details": {
            "name": "Windows",
            "version": "10.0.19041"
        }
    }
}

@pytest.fixture
def sample_alert() -> Dict[Any, Any]:
    """Fixture providing sample Linux alert data."""
    return SAMPLE_ALERT_DATA.copy()

@pytest.fixture
def sample_alert_windows() -> Dict[Any, Any]:
    """Fixture providing sample Windows alert data."""
    return SAMPLE_ALERT_DATA_WINDOWS.copy()

@pytest.fixture
def api_base_url() -> str:
    """Fixture providing the base API URL."""
    return "http://localhost:8000"

@pytest.fixture
def graphql_endpoint(api_base_url: str) -> str:
    """Fixture providing the GraphQL endpoint URL."""
    return f"{api_base_url}/graphql"

@pytest.fixture
def rest_endpoint(api_base_url: str) -> str:
    """Fixture providing the REST API endpoint URL."""
    return f"{api_base_url}/analyze-alert"

@pytest.fixture(scope="session")
def event_loop():
    """Create an instance of the default event loop for the test session."""
    loop = asyncio.get_event_loop_policy().new_event_loop()
    yield loop
    loop.close()

# Test configuration
pytest_plugins = []

def pytest_configure(config):
    """Configure pytest with custom markers."""
    config.addinivalue_line(
        "markers", "integration: mark test as integration test"
    )
    config.addinivalue_line(
        "markers", "slow: mark test as slow running"
    )
    config.addinivalue_line(
        "markers", "graphql: mark test as GraphQL specific"
    )
    config.addinivalue_line(
        "markers", "rest: mark test as REST API specific"
    )
