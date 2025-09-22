# Integration Examples for Trellix WISE GraphQL API

This document shows different ways to integrate the Trellix WISE API with your project and pass alert data at runtime.

## 🔧 Integration Methods

### Method 1: GraphQL with Runtime Data

```python
import httpx
import json
import asyncio

async def analyze_alert_graphql(alert_data_dict, language="en"):
    """
    Analyze alert using GraphQL with runtime data
    """
    url = "http://localhost:8000/graphql"
    
    # Convert alert data to JSON string
    alert_json_string = json.dumps(alert_data_dict)
    
    mutation = """
    mutation AnalyzeAlertData($alertData: String!, $languageIsoCode: String!) {
        analyzeAlertData(alertData: $alertData, languageIsoCode: $languageIsoCode) {
            success
            generatedText
            error
        }
    }
    """
    
    variables = {
        "alertData": alert_json_string,
        "languageIsoCode": language
    }
    
    payload = {
        "query": mutation,
        "variables": variables
    }
    
    async with httpx.AsyncClient(timeout=300.0) as client:
        response = await client.post(url, json=payload)
        result = response.json()
        
        if "errors" in result:
            return {"success": False, "error": result["errors"]}
        else:
            return result["data"]["analyzeAlertData"]

# Example usage
alert_data = {
    "details": [],
    "route": "/hx/api/v3/alerts/id/full_details",
    "data": {
        "_id": 1421,
        "agent": {
            "_id": "abc123",
            "containment_state": "normal"
        },
        # ... your alert data here
    }
}

# Run analysis
result = asyncio.run(analyze_alert_graphql(alert_data, "en"))
print(result)
```

### Method 2: REST API (Easiest Integration)

```python
import requests
import json

def analyze_alert_rest(alert_data_dict, language="en"):
    """
    Analyze alert using REST API - simplest integration method
    """
    url = "http://localhost:8000/analyze-alert"
    
    payload = {
        "alertData": alert_data_dict,
        "languageIsoCode": language
    }
    
    response = requests.post(url, json=payload, timeout=300)
    
    if response.status_code == 200:
        return response.json()
    else:
        return {
            "success": False, 
            "error": f"HTTP {response.status_code}: {response.text}"
        }

# Example usage
alert_data = {
    "_id": 1421,
    "agent": {
        "_id": "xyz789",
        "containment_state": "normal"
    },
    "event_values": {
        "detections": {
            "detection": [{
                "infection": {
                    "infection-name": "Malware.Generic"
                },
                "infected-object": {
                    "file-object": {
                        "file-path": "/tmp/suspicious.exe"
                    }
                }
            }]
        }
    }
    # ... rest of your alert data
}

# Analyze
result = analyze_alert_rest(alert_data, "en")

if result["success"]:
    analysis = json.loads(result["analysis"])
    print("Summary:", analysis["summary"])
    print("Key Points:", analysis["keyPoints"])
else:
    print("Error:", result["error"])
```

### Method 3: Direct Function Call (Same Process)

```python
import json
from graphql_api import generate_text_with_vertex_ai
from jinja2 import Template

async def analyze_alert_direct(alert_data_dict, language="en"):
    """
    Direct function call - use if running in same Python process
    """
    
    prompt_template = """
You are an EDR Security Operations Centre Assistant called "Trellix WISE" made by "Trellix"...
[full prompt template here]

**Detections and Events Data:**
```json
{{ alert_data | tojson }}
```

The response should be in the language with ISO code: {{ language_iso_code }}.
"""
    
    template = Template(prompt_template)
    rendered_prompt = template.render(
        alert_data=alert_data_dict,
        language_iso_code=language
    )
    
    generated_text, success, error = await generate_text_with_vertex_ai(rendered_prompt)
    
    return {
        "success": success,
        "analysis": generated_text,
        "error": error
    }
```

## 🚀 Real-World Integration Examples

### Example 1: Webhook Handler

```python
from fastapi import FastAPI, Request
import httpx

app = FastAPI()

@app.post("/webhook/alert")
async def handle_alert_webhook(request: Request):
    """
    Handle incoming alert webhooks and analyze them
    """
    alert_data = await request.json()
    
    # Call Trellix WISE API
    async with httpx.AsyncClient() as client:
        response = await client.post(
            "http://localhost:8000/analyze-alert",
            json={
                "alertData": alert_data,
                "languageIsoCode": "en"
            }
        )
        
        analysis = response.json()
        
        if analysis["success"]:
            # Process the analysis (send to SIEM, create ticket, etc.)
            await send_to_siem(analysis["analysis"])
            return {"status": "analyzed", "analysis": analysis["analysis"]}
        else:
            return {"status": "error", "error": analysis["error"]}
```

### Example 2: Batch Processing

```python
import asyncio
import httpx
from typing import List, Dict

async def analyze_multiple_alerts(alerts: List[Dict], language="en"):
    """
    Analyze multiple alerts concurrently
    """
    async def analyze_single(alert_data):
        async with httpx.AsyncClient(timeout=300) as client:
            response = await client.post(
                "http://localhost:8000/analyze-alert",
                json={
                    "alertData": alert_data,
                    "languageIsoCode": language
                }
            )
            return response.json()
    
    # Process up to 5 alerts concurrently
    semaphore = asyncio.Semaphore(5)
    
    async def bounded_analyze(alert):
        async with semaphore:
            return await analyze_single(alert)
    
    tasks = [bounded_analyze(alert) for alert in alerts]
    results = await asyncio.gather(*tasks, return_exceptions=True)
    
    return results

# Usage
alerts = [
    {"_id": 1, "data": {...}},
    {"_id": 2, "data": {...}},
    # ... more alerts
]

results = asyncio.run(analyze_multiple_alerts(alerts))
```

### Example 3: Database Integration

```python
import sqlite3
import json
import requests
from datetime import datetime

class AlertAnalyzer:
    def __init__(self, db_path="alerts.db"):
        self.db_path = db_path
        self.init_db()
    
    def init_db(self):
        conn = sqlite3.connect(self.db_path)
        conn.execute("""
            CREATE TABLE IF NOT EXISTS alert_analyses (
                id INTEGER PRIMARY KEY,
                alert_id TEXT,
                analysis TEXT,
                language TEXT,
                created_at TIMESTAMP,
                status TEXT
            )
        """)
        conn.commit()
        conn.close()
    
    def analyze_and_store(self, alert_id, alert_data, language="en"):
        # Call API
        response = requests.post(
            "http://localhost:8000/analyze-alert",
            json={
                "alertData": alert_data,
                "languageIsoCode": language
            },
            timeout=300
        )
        
        analysis_result = response.json()
        
        # Store in database
        conn = sqlite3.connect(self.db_path)
        conn.execute("""
            INSERT INTO alert_analyses 
            (alert_id, analysis, language, created_at, status)
            VALUES (?, ?, ?, ?, ?)
        """, (
            alert_id,
            json.dumps(analysis_result),
            language,
            datetime.now(),
            "success" if analysis_result["success"] else "error"
        ))
        conn.commit()
        conn.close()
        
        return analysis_result

# Usage
analyzer = AlertAnalyzer()
result = analyzer.analyze_and_store("ALERT-001", alert_data)
```

## 📊 Data Format Requirements

Your alert data should be a JSON object. The API is flexible and can handle various formats, but here's the expected structure:

```json
{
    "_id": "unique_alert_id",
    "agent": {
        "_id": "agent_identifier",
        "containment_state": "normal|isolated"
    },
    "event_values": {
        "detections": {
            "detection": [{
                "infection": {
                    "infection-name": "malware_name",
                    "confidence-level": "high|medium|low"
                },
                "infected-object": {
                    "file-object": {
                        "file-path": "/path/to/file",
                        "md5sum": "hash",
                        "sha256sum": "hash"
                    }
                }
            }]
        },
        "os-details": {
            "name": "Windows|Linux|macOS",
            "version": "version_info"
        }
    }
}
```

## 🔒 Security Considerations

1. **Authentication**: Add API keys or JWT tokens for production
2. **Rate Limiting**: Implement rate limiting to prevent abuse
3. **Input Validation**: Validate alert data structure
4. **Logging**: Log all analysis requests for audit trails

```python
# Example with API key
headers = {
    "Authorization": "Bearer your-api-key",
    "Content-Type": "application/json"
}

response = requests.post(url, json=payload, headers=headers)
```

## 🚀 Production Deployment Tips

1. **Load Balancer**: Use multiple instances behind a load balancer
2. **Caching**: Cache analysis results for identical alerts
3. **Monitoring**: Monitor API response times and error rates
4. **Scaling**: Use async processing for high volume scenarios

Choose the integration method that best fits your architecture. The REST API (Method 2) is usually the easiest to integrate with existing systems.
