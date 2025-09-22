# AI-Healthops: AI-API

Light‑weight FastAPI service that performs process monitoring and AI based recommendation for better system resource usage ,using Google Vertex AI (Gemini) via a prompt template.

## Features

- REST only (GraphQL removed)
- Vertex AI (Gemini) text generation
- External Jinja2 prompt template: `prompts/alert_analysis_prompt.j2`
- Strict JSON post‑processing (model output parsed to object)
- Prompt management & execution endpoints
- Swagger / OpenAPI UI at `/swagger` (ReDoc at `/redoc`)
- Endpoints:
  - `GET /` root metadata
  - `GET /health` service / Vertex AI status
  - `GET /hello` simple connectivity test
  - `POST /generate-text` free‑form prompt -> model response
  - `POST /analyze-alert` structured alert analysis (returns parsed JSON object)
  - `GET /prompts` list available prompt templates (*.j2)
  - `GET /prompts/{name}` fetch raw template content
  - `POST /prompts/{name}/render` render a template with variables
  - `POST /prompts/{name}/run` render + execute against model (optional JSON parse) with language override

Language override precedence for /prompts/{name}/run:
1. Query param `outputLanguageCode`
2. JSON body field `outputLanguageCode`
3. `variables.outputLanguageCode`
4. Default: `en`

Example forcing English:
```bash
curl -s -X POST "http://localhost:8000/prompts/interactive_draft_email/run?outputLanguageCode=en" \
  -H 'Content-Type: application/json' \
  -d '{"variables": {"username": "user1", "event_json": "{}"}}' | jq
```

Example requesting French (if supported):
```bash
curl -s -X POST "http://localhost:8000/prompts/interactive_draft_email/run?outputLanguageCode=fr" \
  -H 'Content-Type: application/json' \
  -d '{"variables": {"username": "user1", "event_json": "{}"}}' | jq
```

If unsupported code supplied it falls back to English.

## Requirements

- Python 3.10+
- Google Cloud project with Vertex AI enabled
- Service account / ADC credentials available to the process (e.g. `GOOGLE_APPLICATION_CREDENTIALS`)

## Environment Variables (.env supported)

Put these in a `.env` file at repo root (same dir as `main.py`) or export them before running:

```
GOOGLE_CLOUD_PROJECT_ID=your-project-id
GOOGLE_CLOUD_LOCATION=us-central1
# Optional: PORT=8000
```

If using a service account key file:

```
GOOGLE_APPLICATION_CREDENTIALS=/absolute/path/to/key.json
```

## Install

```bash
python -m venv .venv
source .venv/bin/activate //Windows : .venv\Scripts\Activate.ps1
pip install -r requirements.txt
```

 **Configure Google Cloud Authentication:**
bash
   gcloud auth application-default login
   gcloud config set project YOUR_PROJECT_ID //gcloud config set project svc-hackathon-prod57
   //gcloud auth application-default set-quota-project svc-hackathon-prod57

## Run

```bash
python main.py
# or specify a different port
PORT=8080 python main.py
```

Uvicorn will listen on `0.0.0.0:PORT` (default 8000).

### Interactive API Docs (Swagger / ReDoc)
Once running:
- Swagger UI: http://localhost:8000/swagger
- ReDoc UI:   http://localhost:8000/redoc
- Raw OpenAPI JSON: http://localhost:8000/openapi.json

## Prompt Management Examples

List prompts:
```bash
curl -s http://localhost:8000/prompts | jq
```

Get a prompt:
```bash
curl -s http://localhost:8000/prompts/alert_analysis_prompt | jq -r '.content' | head
```

Render a prompt with variables:
```bash
curl -s -X POST http://localhost:8000/prompts/alert_analysis_prompt/render \
  -H 'Content-Type: application/json' \
  -d '{"variables": {"alert_data": {"events": []}, "language_iso_code": "en"}}' | jq
```

Run a prompt and parse JSON:
```bash
curl -s -X POST http://localhost:8000/prompts/alert_analysis_prompt/run \
  -H 'Content-Type: application/json' \
  -d '{"variables": {"alert_data": {"events": []}, "language_iso_code": "en"}, "parseJson": true}' | jq
```

Field meanings in run response:
- `rendered`: Final prompt sent to model
- `output`: If parseJson=true and success, parsed JSON object; else raw string
- `parsed`: (duplicate) parsed JSON object when available
- `raw`: Always original raw model text when parseJson=true (for debugging)
- `error`: Reason for failure if success=false

## Endpoint Usage (Core)

### 1. Health

```bash
curl -s http://localhost:8000/health | jq
```

### 2. Generate Free‑Form Text

```bash
curl -s -X POST http://localhost:8000/generate-text \
  -H 'Content-Type: application/json' \
  -d '{"prompt": "Summarize zero trust architecture."}' | jq
```

### 3. Analyze Alert Data

```bash
curl -s -X POST http://localhost:8000/analyze-alert \
  -H 'Content-Type: application/json' \
  -d '{"alertData": {"events": [{"process": "cmd.exe", "pid": 1011}]}, "languageIsoCode": "en"}' | jq
```

## Error Handling
- 400: Invalid request body / non‑serializable alertData
- `success=false` + `raw` / `error` indicates model or parse issue

## Development
Auto‑reload:
```bash
uvicorn main:app --reload --port 8000
```

## License
Internal / proprietary (update as needed).
