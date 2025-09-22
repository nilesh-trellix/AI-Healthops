# AI-Hackathon: AI-API

A sophisticated API for monitoring processes and suggesting solutions for better resource utilization using Google Vertex AI. This service provides both REST and GraphQL endpoints for real-time alert analysis, utilizing Jinja2 templates for structured prompt generation and Google's Gemini model for intelligent threat assessment.

## Features

- **Dual API Interface**: Both REST and GraphQL endpoints for flexible integration
- **Real-time Analysis**: Process alert data passed directly via API calls
- **AI-Powered**: Leverages Google Vertex AI (Gemini 2.5-flash-lite) for intelligent analysis
- **Template-Based**: Uses Jinja2 templates for consistent, structured prompt generation
- **Production Ready**: Comprehensive error handling, logging, and environment configuration

## Prerequisites

- Python 3.12 or higher
- Google Cloud Platform account with Vertex AI enabled
- Google Cloud CLI (`gcloud`) installed and authenticated

## Installation

1. **Clone the repository And run:**
   ```bash
   cd trellix-wise
   ```

2. **Set up Python virtual environment:**
   ```bash
   python3 -m venv venv
   source venv/bin/activate  # On Windows: venv\Scripts\activate
   ```

3. **Install dependencies:**
   ```bash
   pip install -r requirements.txt
   ```

4. **Configure Google Cloud Authentication:**
   ```bash
   gcloud auth application-default login
   gcloud config set project YOUR_PROJECT_ID
   ```

5. **Set up environment variables:**
   Create a `.env` file in the root directory:
   ```env
   GOOGLE_CLOUD_PROJECT=your-project-id
   GOOGLE_CLOUD_LOCATION=us-central1
   VERTEX_AI_MODEL=gemini-2.5-flash-lite
   ```

## Running the Service

### Start the API Server

```bash
uvicorn main:app --host 0.0.0.0 --port 8000 --reload
```

The service will be available at:
- **API Server**: http://localhost:8000
- **GraphQL Playground**: http://localhost:8000/graphql
- **API Documentation**: http://localhost:8000/docs

## API Usage

### REST API

**Endpoint**: `POST /analyze-alert`

**Request Body**:
```json
{
  "alert_data": {
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
  },
  "language_iso_code": "en"
}
```

**Response**:
```json
{
  "success": true,
  "generated_text": "{\n  \"summary\": \"Malware Detection Alert\",\n  \"recommendations\": [...],\n  \"mitre_mapping\": [...]\n}",
  "error": null
}
```

**Example using curl**:
```bash
curl -X POST "http://localhost:8000/analyze-alert" \
  -H "Content-Type: application/json" \
  -d '{
    "alert_data": {
      "_id": 1420,
      "agent": {"_id": "test-agent-123", "containment_state": "normal"},
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
        "os-details": {"name": "Linux", "version": "5.15.0-40-generic"}
      }
    },
    "language_iso_code": "en"
  }'
```

### GraphQL API

**Endpoint**: `POST /graphql`

**Query**:
```graphql
mutation AnalyzeAlertData($alertData: String!, $languageIsoCode: String!) {
  analyzeAlertData(alertData: $alertData, languageIsoCode: $languageIsoCode) {
    success
    generatedText
    error
  }
}
```

**Variables**:
```json
{
  "alertData": "{\"_id\":1420,\"agent\":{\"_id\":\"test-agent-123\",\"containment_state\":\"normal\"},\"event_values\":{\"detections\":{\"detection\":[{\"infection\":{\"infection-name\":\"BD.TestSignature\",\"confidence-level\":\"high\"},\"infected-object\":{\"file-object\":{\"file-path\":\"/tmp/suspicious.tar.gz\",\"md5sum\":\"54d9169d56f98d1f35b38a1c29244803\"}}}]},\"os-details\":{\"name\":\"Linux\",\"version\":\"5.15.0-40-generic\"}}}",
  "languageIsoCode": "en"
}
```

## Testing

### Run Test Suite

```bash
# Simple test runner (recommended)
python run_tests.py

# Run individual test files
python tests/test_client.py
python tests/test_runtime_integration.py

# Using pytest (if available)
pytest tests/ -v
pytest tests/ -m graphql     # GraphQL tests only
pytest tests/ -m rest        # REST API tests only
pytest tests/ -m "not slow"  # Skip slow AI tests
```

### Manual Testing

1. **Start the service**:
   ```bash
   uvicorn main:app --reload
   ```

2. **Test REST API**:
   ```bash
   curl -X POST "http://localhost:8000/analyze-alert" \
     -H "Content-Type: application/json" \
     -d '{"alert_data": {...}, "language_iso_code": "en"}'
   ```

3. **Test GraphQL API**:
   Visit http://localhost:8000/graphql and use the GraphQL Playground

## Project Structure

```
trellix-wise/
├── main.py                           # FastAPI server with REST and GraphQL endpoints
├── graphql_api.py                    # GraphQL schema and Vertex AI integration
├── run_tests.py                      # Simple test runner script
├── requirements.txt                  # Python dependencies
├── pytest.ini                       # Pytest configuration
├── .env                             # Environment configuration
├── README.md                        # This file
├── INTEGRATION_GUIDE.md             # Detailed integration documentation
├── FINAL_CLEANUP_SUMMARY.md         # Cleanup process documentation
├── tests/                           # Test suite
│   ├── __init__.py                  # Test package initialization
│   ├── conftest.py                  # Shared test configuration and fixtures
│   ├── test_client.py               # Basic API testing
│   ├── test_runtime_integration.py  # Comprehensive integration tests
│   └── README.md                    # Testing documentation
└── prompt/                          # Jinja2 templates
    ├── dossier_detections_human_prompt.j2
    ├── dossier_detections_system_prompt.j2
    ├── dossier_traces_human_prompt.j2
    └── dossier_traces_system_prompt.j2
```

## Integration Guide

### Basic Integration

```python
import httpx
import json

async def analyze_alert(alert_data, language_code="en"):
    """Analyze alert using REST API"""
    async with httpx.AsyncClient() as client:
        response = await client.post(
            "http://localhost:8000/analyze-alert",
            json={
                "alert_data": alert_data,
                "language_iso_code": language_code
            }
        )
        return response.json()

# Example usage
alert = {
    "_id": 1420,
    "agent": {"_id": "agent-123", "containment_state": "normal"},
    "event_values": {
        "detections": {"detection": [...]},
        "os-details": {"name": "Linux", "version": "5.15.0"}
    }
}

result = await analyze_alert(alert)
print(result["generated_text"])
```

### Batch Processing

```python
async def batch_analyze_alerts(alerts):
    """Process multiple alerts concurrently"""
    async with httpx.AsyncClient() as client:
        tasks = []
        for alert in alerts:
            task = client.post(
                "http://localhost:8000/analyze-alert",
                json={"alert_data": alert, "language_iso_code": "en"}
            )
            tasks.append(task)
        
        responses = await asyncio.gather(*tasks)
        return [r.json() for r in responses]
```

## Troubleshooting

### Common Issues

1. **Vertex AI Authentication Error**:
   ```bash
   gcloud auth application-default login
   gcloud config set project YOUR_PROJECT_ID
   ```

2. **Module Import Errors**:
   ```bash
   pip install -r requirements.txt
   ```

3. **Port Already in Use**:
   ```bash
   uvicorn main:app --port 8001  # Use different port
   ```

### Debug Mode

Enable debug logging by setting environment variable:
```bash
export LOG_LEVEL=DEBUG
uvicorn main:app --reload
```

### Environment Variables

Required variables in `.env`:
```env
GOOGLE_CLOUD_PROJECT=your-project-id
GOOGLE_CLOUD_LOCATION=us-central1
VERTEX_AI_MODEL=gemini-2.5-flash-lite
```

## Performance Considerations

- **Concurrent Requests**: FastAPI handles concurrent requests efficiently
- **Response Time**: Vertex AI calls typically take 2-5 seconds
- **Rate Limits**: Google Cloud Vertex AI has default rate limits
- **Memory Usage**: Each request loads templates into memory temporarily

## Security Notes

- **Authentication**: Relies on Google Cloud Application Default Credentials
- **API Access**: No built-in authentication - implement as needed
- **Data Privacy**: Alert data is sent to Google Vertex AI for processing
- **Environment Variables**: Store sensitive configuration in `.env` file

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests for new functionality
5. Submit a pull request

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Support

For questions or issues:
1. Check the troubleshooting section
2. Review the integration guide
3. Check existing GitHub issues
4. Create a new issue with detailed information

---

**Note**: This service processes cybersecurity alerts using AI. Ensure compliance with your organization's data handling and AI usage policies.
