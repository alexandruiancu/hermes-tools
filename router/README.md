# AI Router - Boost.Beast Implementation

## Overview
C++ HTTP proxy router using Boost.Beast for AI consumption tracking across endpoints.

## Features
- Request routing to multiple backends
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
      "port": 11500,
      "path_prefix": "/llama"
    },
    "firecrawl": {
      "host": "op.local",
      "port": 3001,
      "path_prefix": "/firecrawl"
    },
    "image-gen": {
      "host": "op.local",
      "port": 11903,
      "path_prefix": "/image"
    },
    "searxng": {
      "host": "op.local",
      "port": 0,
      "path_prefix": "/searx",
      "enabled": false
    },
    "openrouter": {
      "host": "openrouter.ai",
      "port": 443,
      "path_prefix": "/openrouter",
      "ssl": true
    }
  },
  "api_routes": {
    "llama-cpp": [
      "/v1/models",
      "/v1/chat/completions",
      "/v1/embeddings",
      "/v1/completions"
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
- **endpoints**: Define backend services with host, port, path prefix, SSL flag, and enabled/disabled status
- **api_routes**: Map API routes to specific endpoints (e.g., `/v1/models` → llama-cpp)
- **server**: Configure the router's listening port and host

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
