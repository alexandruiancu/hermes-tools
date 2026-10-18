# AI Router - Boost.Beast Implementation

## Overview
C++ HTTP proxy router using Boost.Beast for AI consumption tracking across endpoints.  
The router is **transparent** — clients use standard OpenAI-compatible paths (`/v1/models`, `/v1/chat/completions`, etc.) and the router dispatches to the correct backend based on the model name.

## Features
- Transparent proxy — no path prefixes needed
- Model-name-based routing: `endpoint:model` convention
- Multi-backend `/v1/models` aggregation with prefixed model IDs
- Metrics collection: request count, latency, token usage
- Health checks and failover
- Authentication support
- **External configuration via JSON file**

## Build
```bash
cd router
mkdir build && cd build
cmake ..
make
```

## Run
```bash
./ai_router
```

Server runs on port 8080 by default.

## Configuration
The router uses an external JSON configuration file (`config.json`) to define endpoints, API routes, and server settings. This allows you to modify the configuration **without recompiling** the binary.

### Running with a custom config file
```bash
./ai_router /path/to/custom_config.json
```

### Configuration File Format (`config.json`)
```json
{
  "endpoints": {
    "llama-cpp": {
      "host": "op.local",
      "port": 11500
    },
    "firecrawl": {
      "host": "op.local",
      "port": 3002
    },
    "image-gen": {
      "host": "op.local",
      "port": 11903
    },
    "searxng": {
      "host": "op.local",
      "port": 0,
      "enabled": false
    },
    "openrouter": {
      "host": "openrouter.ai",
      "port": 443,
      "ssl": true
    }
  },
  "api_routes": {
    "llama-cpp": [
      "/v1/models",
      "/v1/chat/completions",
      "/v1/embeddings",
      "/v1/completions",
      "/v1/metrics"
    ],
    "firecrawl": [
      "/api/v1/search",
      "/api/v1/scrape"
    ],
    "image-gen": [
      "/v1/images/generations",
      "/v1/images/variations"
    ],
    "openrouter": [
      "/v1/chat/completions",
      "/v1/models"
    ]
  },
  "server": {
    "port": 8080,
    "host": "0.0.0.0"
  }
}
```

### Configuration Sections
- **endpoints**: Define backend services with host, port, SSL flag, and enabled/disabled status
- **api_routes**: Map API routes to specific endpoints (e.g., `/v1/models` → llama-cpp)
- **server**: Configure the router's listening port and host

## Model Name Convention

The router uses a **`endpoint:model`** naming convention to dispatch requests to the correct backend.

### How it works

1. **`GET /v1/models`** — The router queries all backends that list `/v1/models` in their `api_routes`, collects their model lists, and prefixes each model `id` with the endpoint name:
   ```json
   {
     "data": [
       { "id": "llama-cpp:Qwen3.6-35B", ... },
       { "id": "openrouter:claude-3-opus", ... }
     ]
   }
   ```

2. **`POST /v1/chat/completions`** — The client sends the prefixed model name:
   ```json
   {
     "model": "llama-cpp:Qwen3.6-35B",
     "messages": [...]
   }
   ```
   The router extracts `llama-cpp` as the target endpoint, strips the prefix from the body before forwarding, and sends the request to the llama-cpp backend with `"model": "Qwen3.6-35B"`.

3. **Non-model routes** (e.g., `/api/v1/search`) — The router dispatches based on `api_routes` matching, unchanged from before.

## Metrics

### Example Request
```bash
curl -s "http://op.local:11500/v1/chat/completions" \
  -H "Content-Type: application/json" \
  -d '{"model":"Qwen3.6-35B","messages":[{"role":"user","content":"metrics"}],"max_tokens":100}'
```

### Response Metrics
```json
{
  "timings": {
    "cache_n": 0,
    "prompt_n": 11,
    "prompt_ms": 39.007,
    "prompt_per_token_ms": 3.546,
    "prompt_per_second": 282.001,
    "predicted_n": 100,
    "predicted_ms": 500.851,
    "predicted_per_token_ms": 5.059,
    "predicted_per_second": 197.664,
    "draft_n": 78,
    "draft_n_accepted": 59
  }
}
```

Metrics are returned per-request in the `timings` field of each `/v1/chat/completions` response.
Metrics are logged to stdout and can be extended to Prometheus export.
