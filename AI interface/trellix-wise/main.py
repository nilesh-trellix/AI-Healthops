from fastapi import FastAPI, HTTPException, Query
from pathlib import Path
from jinja2 import Template
from typing import Optional, Tuple, List, Dict, Any
import os, json, re
from pydantic import BaseModel, Field
import hashlib
import datetime

# Vertex AI deps
from vertexai.generative_models import GenerativeModel
from vertexai import init as vertex_init

# Load environment variables from .env file
env_file = Path(__file__).parent / ".env"
if env_file.exists():
    with open(env_file, 'r') as f:
        for line in f:
            if line.strip() and not line.startswith('#'):
                key, value = line.strip().split('=', 1)
                os.environ[key] = value

# Vertex AI init
PROJECT_ID = os.environ.get("GOOGLE_CLOUD_PROJECT_ID")
LOCATION = os.environ.get("GOOGLE_CLOUD_LOCATION", "us-central1")
MODEL_NAME = "gemini-2.5-flash-lite"
try:
    if PROJECT_ID:
        vertex_init(project=PROJECT_ID, location=LOCATION)
except Exception as e:  # pragma: no cover
    print(f"Vertex AI init error: {e}")

app = FastAPI(
    title="Trellix WISE REST API",
    description="REST API for cybersecurity alert analysis using Google Vertex AI",
    version="1.0.0",
    docs_url="/swagger",
    redoc_url="/redoc",
    openapi_url="/openapi.json"
)

# Prompt template cache
TEMPLATE_PATH = Path(__file__).parent / "prompts" / "alert_analysis_prompt.j2"
_template_cache: Optional[str] = None

def load_alert_template() -> str:
    global _template_cache
    if _template_cache is None:
        try:
            _template_cache = TEMPLATE_PATH.read_text(encoding="utf-8")
        except FileNotFoundError:
            _template_cache = ""
    return _template_cache

def render_alert_prompt(alert_data: dict, language_iso_code: str) -> str:
    tmpl = load_alert_template()
    return Template(tmpl).render(alert_data=alert_data, language_iso_code=language_iso_code)

async def generate_text_with_vertex_ai(prompt: str) -> Tuple[str, bool, Optional[str]]:
    try:
        model = GenerativeModel(MODEL_NAME)
        json_prompt = f"{prompt}\n\nIMPORTANT: Return your response as valid JSON only."
        resp = model.generate_content(json_prompt)
        if hasattr(resp, 'text') and resp.text:
            return resp.text, True, None
        return "", False, "Empty response from model"
    except Exception as e:
        return "", False, str(e)

# --------- JSON extraction helpers ---------
JSON_BLOCK_REGEX = re.compile(r"\{[\s\S]*\}")

def extract_json_block(text: str) -> Tuple[Optional[dict], Optional[str]]:
    if not text:
        return None, "empty output"
    cleaned = text.strip()
    # strip fences
    cleaned = re.sub(r"```(?:json)?", "", cleaned, flags=re.IGNORECASE).strip('`\n ')
    # First direct attempt
    try:
        return json.loads(cleaned), None
    except Exception:
        pass
    # Find first { ... }
    m = JSON_BLOCK_REGEX.search(cleaned)
    if m:
        candidate = m.group(0)
        try:
            return json.loads(candidate), None
        except Exception as e:
            return None, f"candidate parse error: {e}"
    return None, "no JSON object found"

# --------- Pydantic Models ---------
class GenerateTextRequest(BaseModel):
    prompt: str = Field(..., description="Prompt text to send to the model")

class AnalyzeAlertRequest(BaseModel):
    alertData: dict = Field(..., description="Raw alert data object")
    languageIsoCode: Optional[str] = Field("en", description="ISO language code for output")

class AnalyzeAlertResponse(BaseModel):
    success: bool
    analysis: Optional[dict] = None
    raw: Optional[str] = None
    language: Optional[str] = None
    error: Optional[str] = None

class RenderPromptRequest(BaseModel):
    variables: Dict[str, object] = Field(default_factory=dict, description="Variables available to the Jinja template")

class RunPromptRequest(BaseModel):
    variables: Dict[str, Any] = Field(default_factory=dict)
    parseJson: bool = Field(True, description="Attempt to parse model output as JSON")
    outputLanguageCode: Optional[str] = Field(None, description="Desired output language ISO code (overridden by query param if provided)")

class RunPromptResponse(BaseModel):
    success: bool
    rendered: Optional[str] = None
    output: Optional[object] = None  # parsed JSON if parseJson=true & success, else raw string
    raw: Optional[str] = None        # original raw model text when parseJson=true
    error: Optional[str] = None

# Dedupe helpers placed early so available to all endpoints

def _dedupe_list_of_objects(items):
    if not isinstance(items, list):
        return items
    seen = set()
    out = []
    for obj in items:
        try:
            key_src = json.dumps(obj, sort_keys=True)
        except Exception:
            key_src = str(obj)
        h = hashlib.sha256(key_src.encode('utf-8')).hexdigest()
        if h not in seen:
            seen.add(h)
            out.append(obj)
    return out

def dedupe_analysis(analysis: dict) -> dict:
    if not isinstance(analysis, dict):
        return analysis
    for k in ["keyPoints", "recommendations", "mitreInfo", "knownBreaches"]:
        if k in analysis:
            analysis[k] = _dedupe_list_of_objects(analysis.get(k))
    return analysis

SUPPORTED_LANG_CODES = {"en", "es", "fr", "de", "it", "pt", "ja", "ko", "zh", "pl"}

def normalize_lang(code: Optional[str]) -> str:
    if not code:
        return "en"
    c = code.strip().lower()
    if c.startswith("eng"): return "en"
    return c if c in SUPPORTED_LANG_CODES else "en"

@app.get("/")
async def root():
    return {
        "message": "Trellix WISE REST API",
        "description": "Cybersecurity alert analysis",
        "version": "1.0.0",
        "endpoints": {
            "health": "/health",
            "analyze_alert_rest": "/analyze-alert",
            "generate_text": "/generate-text",
            "hello": "/hello",
            "list_prompts": "/prompts",
            "get_prompt": "/prompts/{name}",
            "render_prompt": "/prompts/{name}/render",
            "run_prompt": "/prompts/{name}/run"
        }
    }

@app.get("/hello")
async def hello():
    return {"message": "Hello from Trellix WISE REST API"}

@app.get("/health")
async def health_check():
    return {
        "status": "healthy",
        "service": "Trellix WISE REST API",
        "vertex_ai_project": PROJECT_ID or "Not configured",
        "vertex_ai_location": LOCATION
    }

@app.post("/generate-text")
async def generate_text_endpoint(payload: GenerateTextRequest):
    text, ok, err = await generate_text_with_vertex_ai(payload.prompt)
    if ok:
        return {"generated_text": text, "success": True}
    return {"generated_text": "", "success": False, "error": err}

@app.post("/analyze-alert", response_model=AnalyzeAlertResponse)
async def analyze_alert_rest(payload: AnalyzeAlertRequest):
    alert_data = payload.alertData
    language = payload.languageIsoCode or "en"
    # JSON serializable check
    try:
        json.dumps(alert_data)
    except Exception as e:
        return AnalyzeAlertResponse(success=False, error=f"alertData not JSON serializable: {e}")

    if not load_alert_template():
        return AnalyzeAlertResponse(success=False, error="Prompt template missing")

    prompt = render_alert_prompt(alert_data, language)
    raw_text, ok, err = await generate_text_with_vertex_ai(prompt)
    print("DEBUG model ANalysis output:", repr(raw_text))
    if not ok:
        return AnalyzeAlertResponse(success=False, error=err, raw=raw_text, language=language)

    parsed, perr = extract_json_block(raw_text)
    if parsed is not None:
        parsed = dedupe_analysis(parsed)
        return AnalyzeAlertResponse(success=True, analysis=parsed, language=language)
    return AnalyzeAlertResponse(success=False, error=f"Model output not valid JSON: {perr}", raw=raw_text, language=language)

# --------- Prompt discovery helpers ---------
PROMPTS_DIR = TEMPLATE_PATH.parent

def list_prompt_files() -> List[str]:
    if not PROMPTS_DIR.exists():
        return []
    return sorted([p.stem for p in PROMPTS_DIR.glob('*.j2')])

def load_named_prompt(name: str) -> str:
    safe = name.replace('..', '').replace('/', '')
    path = PROMPTS_DIR / f"{safe}.j2"
    if not path.exists() or not path.is_file():
        raise FileNotFoundError(name)
    return path.read_text(encoding='utf-8')

@app.get("/prompts", response_model=List[str])
async def list_prompts():
    return list_prompt_files()

@app.get("/prompts/{name}")
async def get_prompt(name: str):
    try:
        content = load_named_prompt(name)
        return {"name": name, "content": content}
    except FileNotFoundError:
        raise HTTPException(status_code=404, detail="Prompt not found")

@app.post("/prompts/{name}/render")
async def render_prompt(name: str, req: RenderPromptRequest):
    try:
        raw = load_named_prompt(name)
    except FileNotFoundError:
        raise HTTPException(status_code=404, detail="Prompt not found")
    rendered = Template(raw).render(**req.variables)
    return {"name": name, "rendered": rendered}


def save_output_to_json_file(txt: str, prompt_name: str, success: bool, rendered_prompt: str = "") -> str:
    """Save model output to a timestamped JSON file and return the filename."""
    timestamp = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
    status = "success" if success else "error"
    filename = f"output_{prompt_name}_{status}_{timestamp}.json"
    txt_filename = f"output_{prompt_name}_{status}_{timestamp}.txt"
    
    # Create outputs directory if it doesn't exist
    output_dir = Path(__file__).parent / "outputs"
    output_dir.mkdir(exist_ok=True)
    
    filepath = output_dir / filename
    txt_filepath = output_dir / txt_filename
    
    # Create JSON structure
    output_data = {
        "timestamp": datetime.datetime.now().isoformat(),
        "prompt_name": prompt_name,
        "status": status,
        "success": success,
        "rendered_prompt": rendered_prompt,
        "model_response": txt,
        "response_length": len(txt) if txt else 0
    }
    
    try:
        # Save JSON file
        with open(filepath, 'w', encoding='utf-8') as f:
            json.dump(output_data, f, indent=2, ensure_ascii=False, separators=(',', ': '))
        
        # Save plain text file with proper newlines
        with open(txt_filepath, 'w', encoding='utf-8') as f:
            f.write(f"Timestamp: {datetime.datetime.now().isoformat()}\n")
            f.write(f"Prompt: {prompt_name}\n")
            f.write(f"Status: {status}\n")
            f.write("-" * 50 + "\n\n")
            f.write(txt if txt else "No content")
        
        return str(filepath)
    except Exception as e:
        print(f"Failed to save output to files: {e}")
        return ""

@app.post("/prompts/{name}/run", response_model=RunPromptResponse)
async def run_prompt(
    name: str,
    req: RunPromptRequest,
    outputLanguageCode: Optional[str] = Query(None, description="Override output language (ISO code, default en)")
):
    try:
        raw = load_named_prompt(name)
    except FileNotFoundError:
        return RunPromptResponse(success=False, error="Prompt not found")
    # Resolve final language precedence: query > body field > variables > default
    final_lang = outputLanguageCode or req.outputLanguageCode or req.variables.get("outputLanguageCode") or "en"
    final_lang = normalize_lang(final_lang)
    req.variables["outputLanguageCode"] = final_lang

    rendered = Template(raw).render(**req.variables)
    
    #print("DEBUG model output:", repr(rendered))
    txt, ok, err = await generate_text_with_vertex_ai(rendered)

    # Save the output to JSON file
    saved_file = save_output_to_json_file(txt, name, ok, rendered)
    if saved_file:
        print(f"Model output saved to JSON: {saved_file}")
    
    if not ok:
        return RunPromptResponse(success=False, rendered=rendered, output=txt, raw=txt if req.parseJson else None, error=err)
    if req.parseJson:
        parsed, perr = extract_json_block(txt)
        if parsed is None:
            return RunPromptResponse(success=False, rendered=rendered, output=txt, raw=txt, error=f"Parse error Prompt: {perr}")
        parsed = dedupe_analysis(parsed)
        return RunPromptResponse(success=True, rendered=rendered, output=parsed, raw=txt)
    return RunPromptResponse(success=True, rendered=rendered, output=txt)

if __name__ == "__main__":
    port = int(os.environ.get('PORT', 8000))
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=port)
