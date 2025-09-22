#!/usr/bin/env python3
"""
Test script demonstrating runtime alert data integration with Trellix WISE API
"""

import httpx
import json
import asyncio

# Sample alert data that you would get from your system at runtime
SAMPLE_ALERT_DATA = {
    "details": [],
    "route": "/hx/api/v3/alerts/id/full_details",
    "data": {
        "_id": 9999,
        "agent": {
            "_id": "test-agent-123",
            "url": "/hx/api/v3/hosts/test-agent-123",
            "containment_state": "normal"
        },
        "appliance": {
            "_id": "TEST123ABC"
        },
        "event_at": "2025-08-02T10:30:00.000Z",
        "matched_at": "2025-08-02T10:30:00.000Z",
        "reported_at": "2025-08-02T10:30:05.000Z",
        "source": "MAL",
        "subtype": "AV",
        "resolution": "ALERT",
        "is_false_positive": False,
        "event_values": {
            "system-data": {
                "timestamp": "2025-08-02T10:30:00.000Z",
                "product-version": "1.0"
            },
            "os-details": {
                "name": "Windows",
                "version": "Windows 10 Pro",
                "os-arch": "x64"
            },
            "detections": {
                "detection": [{
                    "engine": {
                        "engine-type": "av",
                        "engine-version": "3.0"
                    },
                    "infected-object": {
                        "object-type": "file",
                        "file-object": {
                            "file-path": "C:\\Users\\testuser\\Downloads\\suspicious.exe",
                            "md5sum": "abc123def456789",
                            "sha256sum": "def789abc123456",
                            "size-in-bytes": "2048"
                        }
                    },
                    "infection": {
                        "confidence-level": "high",
                        "infection-type": "malware",
                        "infection-name": "Trojan.Generic.Test"
                    },
                    "action": {
                        "requested-action": "quarantine",
                        "applied-action": "quarantine",
                        "result": "success"
                    }
                }]
            }
        }
    },
    "message": "OK"
}

async def test_graphql_runtime_data():
    """Test GraphQL API with runtime alert data"""
    print("🧪 Testing GraphQL API with runtime alert data...")
    
    url = "http://localhost:8000/graphql"
    
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
        "alertData": json.dumps(SAMPLE_ALERT_DATA),
        "languageIsoCode": "en"
    }
    
    payload = {
        "query": mutation,
        "variables": variables
    }
    
    try:
        async with httpx.AsyncClient(timeout=300.0) as client:
            response = await client.post(url, json=payload)
            result = response.json()
            
            if "errors" in result:
                print("❌ GraphQL Errors:")
                for error in result["errors"]:
                    print(f"   {error['message']}")
                return False
            else:
                data = result["data"]["analyzeAlertData"]
                print(f"✅ GraphQL Success: {data['success']}")
                
                if data["success"]:
                    analysis = json.loads(data["generatedText"])
                    print("📊 Analysis Summary:")
                    print(f"   {analysis.get('summary', 'No summary available')[:100]}...")
                    print(f"📈 Key Points: {len(analysis.get('keyPoints', []))}")
                    print(f"💡 Recommendations: {len(analysis.get('recommendations', []))}")
                    return True
                else:
                    print(f"❌ Analysis Error: {data['error']}")
                    return False
                    
    except Exception as e:
        print(f"❌ Request Error: {e}")
        return False

async def test_rest_api():
    """Test REST API with runtime alert data"""
    print("\n🧪 Testing REST API with runtime alert data...")
    
    url = "http://localhost:8000/analyze-alert"
    
    payload = {
        "alertData": SAMPLE_ALERT_DATA,
        "languageIsoCode": "en"
    }
    
    try:
        async with httpx.AsyncClient(timeout=300.0) as client:
            response = await client.post(url, json=payload)
            result = response.json()
            
            if response.status_code == 200 and result["success"]:
                print("✅ REST API Success")
                analysis = json.loads(result["analysis"])
                print("📊 Analysis Summary:")
                print(f"   {analysis.get('summary', 'No summary available')[:100]}...")
                print(f"📈 Key Points: {len(analysis.get('keyPoints', []))}")
                print(f"💡 Recommendations: {len(analysis.get('recommendations', []))}")
                print(f"🌐 Language: {result['language']}")
                return True
            else:
                print(f"❌ REST API Error: {result.get('error', 'Unknown error')}")
                return False
                
    except Exception as e:
        print(f"❌ Request Error: {e}")
        return False

def test_different_languages():
    """Test different language outputs"""
    print("\n🌍 Testing different languages...")
    
    languages = ["en", "es", "fr"]
    
    for lang in languages:
        print(f"\n🗣️  Testing language: {lang}")
        payload = {
            "alertData": SAMPLE_ALERT_DATA,
            "languageIsoCode": lang
        }
        
        try:
            import requests
            response = requests.post(
                "http://localhost:8000/analyze-alert",
                json=payload,
                timeout=300
            )
            
            if response.status_code == 200:
                result = response.json()
                if result["success"]:
                    analysis = json.loads(result["analysis"])
                    print(f"✅ {lang.upper()} - Success")
                    summary = analysis.get('summary', '')[:50] + "..."
                    print(f"   Summary: {summary}")
                else:
                    print(f"❌ {lang.upper()} - Error: {result['error']}")
            else:
                print(f"❌ {lang.upper()} - HTTP Error: {response.status_code}")
                
        except Exception as e:
            print(f"❌ {lang.upper()} - Exception: {e}")

async def main():
    """Run all tests"""
    print("🚀 Trellix WISE API Runtime Integration Tests")
    print("=" * 50)
    
    # Test GraphQL with runtime data
    graphql_success = await test_graphql_runtime_data()
    
    # Test REST API
    rest_success = await test_rest_api()
    
    # Test different languages (synchronous for simplicity)
    test_different_languages()
    
    print("\n📋 Test Summary:")
    print(f"   GraphQL API: {'✅ PASS' if graphql_success else '❌ FAIL'}")
    print(f"   REST API:    {'✅ PASS' if rest_success else '❌ FAIL'}")
    
    if graphql_success or rest_success:
        print("\n🎉 Integration is working! You can now:")
        print("   1. Use the REST API at /analyze-alert for easy integration")
        print("   2. Use the GraphQL mutation analyzeAlertData for more control")
        print("   3. Pass your alert data at runtime instead of using files")
    else:
        print("\n❌ Integration tests failed. Check if the API is running:")
        print("   python main.py")

if __name__ == "__main__":
    asyncio.run(main())
