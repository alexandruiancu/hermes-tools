# Stable‑Diffusion Server (sd_server1)

This repository contains a minimal HTTP server that exposes a single endpoint for generating images using the **stable‑diffusion.cpp** library.  The server is built with **Crow** and uses the C API of stable‑diffusion.cpp.

## Prerequisites

- CMake 3.20+ (used by the parent project)
- A C++20 compiler (gcc 10+, clang 12+)
- `git` and `curl`
- (Optional) CUDA if you want GPU acceleration – the current build is CPU‑only.

## Building

The project is part of the larger `hermes-tools` workspace.  To build the server run:

```bash
cd /home/andu/services/hermes-tools
# Build the whole workspace (this will also build sd_server1)
ninja -j$(nproc)
```

The executable will be located at `build/ai_router`.

## Model file

The server expects a GGUF model file.  A tiny, publicly‑available model is provided in the `models/` directory.  If you want to use a different model, place the `.gguf` file in `sd_server1/models/` and update the `sd_model_path` entry in `build/config.json`.

### Download the tiny model

```bash
mkdir -p sd_server1/models
curl -L -o sd_server1/models/tiny_sd.gguf \
    https://huggingface.co/gsdf/Counterfeit-V2.5/resolve/main/tiny_sd.gguf
```

## Configuration

Create or edit `build/config.json` with the following content:

```json
{
  "server": {
    "port": 8080
  },
  "sd_model_path": "/home/andu/services/hermes-tools/sd_server1/models/tiny_sd.gguf"
}
```

The `sd_model_path` should point to the absolute path of the GGUF file.

## Running the server

```bash
cd /home/andu/services/hermes-tools/build
./ai_router
```

You should see output similar to:

```
Config missing 'endpoints' object
Config missing 'api_routes' object
Configuration loaded from: config.json
AI Router starting on 0.0.0.0:8080
Loaded 0 endpoints
```

The server is now listening on port **8080**.

## API

### POST `/sd`

Generate an image.

**Request body (JSON)**

```json
{
  "prompt": "a sunny beach",
  "width": 512,
  "height": 512,
  "steps": 20,
  "cfg_scale": 7.5
}
```

**Response**

The response body is a PNG image encoded as a base64 string.

### GET `/health`

Health‑check endpoint that returns `OK`.

## Testing

```bash
curl -X POST http://localhost:8080/sd \
     -H "Content-Type: application/json" \
     -d '{"prompt":"a sunny beach","width":512,"height":512,"steps":20,"cfg_scale":7.5}' \
     | base64 | head -c 100
```

You should see a base64 string that starts with `iVBORw0KGgo` (the PNG magic number).

## Notes

- The server currently does not expose any metrics endpoints.  The `Metrics` class is available for future integration.
- The build uses the CPU backend of stable‑diffusion.cpp.  If you have an NVIDIA GPU and the CUDA toolkit installed, you can enable GPU support by setting `params.rng_type = CUDA_RNG` in `sd_server1/src/main.cpp` and linking against the CUDA libraries.

---

Happy generating!
