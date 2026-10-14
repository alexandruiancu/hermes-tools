# AI Router - Boost.Beast Implementation

## Overview
C++ HTTP proxy router using Boost.Beast for AI consumption tracking across endpoints.

## Features
- Request routing to multiple backends
- Metrics collection: request count, latency, token usage
- Health checks and failover
- Authentication support

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

## Backend Configuration
Edit `src/proxy.cpp` to configure backends:
- llama-cpp: op.local:11500
- firecrawl: op.local:3001
- image-gen: op.local:11903
- searxng: TBD
- openrouter: openrouter.ai:443

## Metrics
Metrics are logged to stdout and can be extended to Prometheus export.
