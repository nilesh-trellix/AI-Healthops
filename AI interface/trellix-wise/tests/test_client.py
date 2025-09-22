import httpx
import json
import asyncio

# Sample alert data for testing
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

async def test_graphql_api():
    """
    Test the GraphQL API by calling the analyzeAlertData mutation with runtime data
    """
    # GraphQL endpoint
    url = "http://localhost:8000/graphql"
    
    # GraphQL mutation for runtime data
    mutation = """
    mutation AnalyzeAlertData($alertData: String!, $languageIsoCode: String!) {
        analyzeAlertData(alertData: $alertData, languageIsoCode: $languageIsoCode) {
            success
            generatedText
            error
        }
    }
    """
    
    # Variables
    variables = {
        "alertData": json.dumps(SAMPLE_ALERT_DATA),
        "languageIsoCode": "en"
    }
    
    # Request payload
    payload = {
        "query": mutation,
        "variables": variables
    }
    
    try:
        async with httpx.AsyncClient(timeout=300.0) as client:
            print("Calling GraphQL API with runtime alert data...")
            response = await client.post(url, json=payload)
            
            if response.status_code == 200:
                result = response.json()
                
                if "errors" in result:
                    print("GraphQL Errors:")
                    for error in result["errors"]:
                        print(f"  - {error['message']}")
                else:
                    data = result["data"]["analyzeAlertData"]
                    print(f"Success: {data['success']}")
                    
                    if data["success"]:
                        print("Generated Analysis:")
                        # Try to parse the generated text as JSON for pretty printing
                        try:
                            analysis = json.loads(data["generatedText"])
                            print(json.dumps(analysis, indent=2))
                        except json.JSONDecodeError:
                            print(data["generatedText"])
                    else:
                        print(f"Error: {data['error']}")
            else:
                print(f"HTTP Error: {response.status_code}")
                print(response.text)
                
    except Exception as e:
        print(f"Error calling API: {e}")

async def test_hello_query():
    """
    Test the simple hello query
    """
    url = "http://localhost:8000/graphql"
    
    query = """
    query {
        hello
    }
    """
    
    payload = {"query": query}
    
    try:
        async with httpx.AsyncClient() as client:
            response = await client.post(url, json=payload)
            
            if response.status_code == 200:
                result = response.json()
                print("Hello Query Result:")
                print(result["data"]["hello"])
            else:
                print(f"HTTP Error: {response.status_code}")
                
    except Exception as e:
        print(f"Error: {e}")

if __name__ == "__main__":
    print("Testing Trellix WISE GraphQL API")
    print("=" * 40)
    
    # Test hello query first
    print("\n1. Testing Hello Query:")
    asyncio.run(test_hello_query())
    
    # Test alert analysis
    print("\n2. Testing Alert Analysis:")
    asyncio.run(test_graphql_api())
