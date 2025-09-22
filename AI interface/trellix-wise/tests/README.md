# Trellix WISE API Tests

This directory contains comprehensive tests for the Trellix WISE Alert Analysis API.

## Test Structure

```
tests/
├── __init__.py                    # Package initialization
├── conftest.py                    # Shared test configuration and fixtures
├── test_client.py                 # Basic API testing
├── test_runtime_integration.py    # Comprehensive runtime integration tests
└── README.md                      # This file
```

## Test Files

### `test_client.py`
Basic API testing with sample data:
- Tests GraphQL hello query
- Tests GraphQL alert analysis with runtime data
- Simple smoke tests to verify API functionality

### `test_runtime_integration.py`
Comprehensive runtime integration tests:
- REST API testing with various alert formats
- GraphQL API testing with complex mutations
- Batch processing tests
- Error handling tests
- Performance validation

### `conftest.py`
Shared test configuration:
- Sample alert data fixtures (Linux and Windows)
- API endpoint fixtures
- pytest configuration
- Custom test markers

## Running Tests

### Prerequisites
```bash
# Install test dependencies
pip install -r requirements.txt
```

### Run All Tests
```bash
# Run all tests
pytest tests/

# Run with verbose output
pytest tests/ -v

# Run with coverage
pytest tests/ --cov=. --cov-report=html
```

### Run Specific Tests
```bash
# Run basic client tests
python tests/test_client.py

# Run runtime integration tests
python tests/test_runtime_integration.py

# Run only GraphQL tests
pytest tests/ -m graphql

# Run only REST API tests
pytest tests/ -m rest

# Skip slow tests
pytest tests/ -m "not slow"
```

### Run Integration Tests
```bash
# Run integration tests (requires running API server)
pytest tests/ -m integration
```

## Test Markers

Tests are organized with the following markers:

- `@pytest.mark.integration` - Integration tests requiring running API server
- `@pytest.mark.slow` - Slow-running tests (AI API calls)
- `@pytest.mark.graphql` - GraphQL-specific tests
- `@pytest.mark.rest` - REST API-specific tests

## Test Data

Tests use sample alert data defined in `conftest.py`:

- **SAMPLE_ALERT_DATA**: Linux-based alert with BD.TestSignature detection
- **SAMPLE_ALERT_DATA_WINDOWS**: Windows-based alert with Trojan detection

## API Server Requirement

Most tests require the API server to be running:

```bash
# Start the API server in another terminal
uvicorn main:app --reload
```

## Test Configuration

Configuration is handled through:
- `pyproject.toml` - pytest configuration
- `conftest.py` - fixtures and test data
- Environment variables via `.env` file

## Adding New Tests

1. Create test files following the naming convention `test_*.py`
2. Use fixtures from `conftest.py` for common test data
3. Add appropriate markers for test categorization
4. Follow async/await patterns for API calls

Example test structure:
```python
import pytest
from tests.conftest import SAMPLE_ALERT_DATA

@pytest.mark.integration
@pytest.mark.graphql
async def test_new_feature(graphql_endpoint, sample_alert):
    # Your test code here
    pass
```

## Troubleshooting

### Common Issues

1. **Tests fail with connection errors**:
   - Ensure API server is running on localhost:8000
   - Check that all dependencies are installed

2. **Import errors**:
   - Run tests from project root directory
   - Ensure virtual environment is activated

3. **Async test issues**:
   - Tests use `asyncio_mode = "auto"` in pytest configuration
   - No need for manual event loop handling

### Debug Mode

Run tests with debug output:
```bash
pytest tests/ -v -s --tb=long
```

## Performance Notes

- Tests involving AI API calls may take 5-15 seconds each
- Use `-m "not slow"` to skip AI-dependent tests during development
- Integration tests require network connectivity to Google Vertex AI
