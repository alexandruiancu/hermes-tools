# tiny self-hosted LLM & tools gateway

A small C++ HTTP router (`ai_router`, Boost.Beast) that sits in front of local and cloud AI backends and exposes a single OpenAI‑compatible API. Clients talk to one port; the router dispatches each request to the right engine based on the model name (`endpoint:model`), aggregates `/v1/models` across all backends, and tracks per‑endpoint metrics. Current backends: `llama-cpp` (local LLM engine), `firecrawl` (web search/scrape), `image-gen`, and `openrouter` (cloud, over SSL). Configuration is externalized to `config.json`, so backends can be added or changed without recompiling. The older image‑generation server (`sd_server1`, Crow) is kept as a legacy component.

---

## 1. Directory layout

```
~/services/hermes-tools/
├─ CMakeLists.txt    - top-level build configuration
├─ cmake/            - CPM helpers (CPM.cmake, crow.cmake)
├─ _deps/            - external dependencies (asio, crow, llama.cpp, etc.)
├─ build/            - compiled output and build artifacts
├─ common/           - shared C++ helpers
│  ├─ include/       - header files
│  ├─ src/           - source files
│  ├─ config.json    - common configuration
│  └─ CMakeLists.txt
├─ doc/              - documentation
│  ├─ README_arch.md    - Arch Linux toolbox setup
│  ├─ README_fedora.md  - Fedora toolbox setup
│  ├─ README_ubuntu.md  - Ubuntu Multipass setup
│  ├─ setup_cuda_arch.sh    - Arch Linux toolbox container setup
│  ├─ setup_cuda_fedora.sh  - Fedora 44 toolbox container setup
│  └─ setup_cuda_ubuntu.sh  - Ubuntu 22.04 Multipass container setup
├─ router/           - AI router (Boost.Beast HTTP router)
│  ├─ CMakeLists.txt
│  ├─ README.md
│  ├─ _deps/
│  ├─ src/              - main.cpp, proxy.cpp, metrics.cpp, config_loader.cpp
│  ├─ include/
│  ├─ config.json       - endpoints, api_routes, server settings
│  └─ CMakeLists.txt
├─ sd_server1/       - legacy: Crow image-gen server (requires CUDA)
│  ├─ CMakeLists.txt
│  ├─ include/
│  ├─ models/
│  ├─ src/
│  ├─ CMakePresets.json
│  ├─ CMakeUserPresets.json
│  ├─ Doxyfile
│  ├─ mkdocs.yml
│  ├─ MODULE.bazel
│  ├─ README.md
│  ├─ vcpkg.json
│  ├─ cmake/
│  ├─ examples/
│  └─ tests/
├─ sd_container/     - legacy: container quadlets
│  ├─ .gitignore
│  └─ models/
├─ .env              - runtime parametrization
├─ plan.md           - design notes
├─ sr-integration-plan.md - integration plan summary
└─ .gitignore
```
---

## 2. Crow server (`server.cpp`)
This component is implemented in the sub‑project `sd_server1`. 
It serves as a lightweight HTTP wrapper around the underlying LLM inference binary, exposing an endpoint that mimics the OpenAI Chat Completion API. By forwarding requests to the model through Crow, downstream services can interact with the local LLM using familiar API calls without needing direct access to the binary.

---

## 3. CMake build file (`CMakeLists.txt`)

The top-level `CMakeLists.txt` builds `common` and `router` unconditionally, and pulls `asio` + `crow` via CPM.

**`sd_server1` is built only when `USE_CUDA=ON`.** It links `CUDA::cudart`, `Crow`, and `hermes_common`, so it needs the CUDA toolkit and a C++20 compiler. The router itself is C++17 and does not need CUDA:

```bash
# router only (no CUDA)
cmake -S . -B build
cmake --build build

# router + legacy sd_server1 (requires CUDA)
cmake -S . -B build -DUSE_CUDA=ON
cmake --build build
```

**SSL option.** The router links **OpenSSL** (`OpenSSL::SSL`, `OpenSSL::Crypto`) and requires it (`find_package(OpenSSL REQUIRED)`). This powers the TLS path in `http_forward()`: backends flagged `"ssl": true` (e.g. `openrouter` on port 443) are reached over an `ssl::stream` with SNI, while plain backends use a raw TCP socket. Boost is located in `CONFIG` mode (`find_package(Boost 1.70 REQUIRED CONFIG COMPONENTS system)`), and `nlohmann_json` is fetched via CPM.

---

## 4. Dockerfile

> **Why two stages?** The build stage pulls the C++ compiler and Crow dev libs, compiles the server, then copies only the compiled binary and the `llama-diffusion-cli` binary into the minimal runtime image, keeping the final image size small.

## 5. Toolbox container build & debug

The build and debug operations happen inside a toolbox container. Setup scripts are available:

- **Arch Linux**: `setup_cuda_arch.sh` - creates a toolbox container with CUDA development tools
- **Fedora**: `setup_cuda_toolbox.sh` - creates a Fedora 44 toolbox container named `fedora-44-cuda-dev`
- **Ubuntu**: `setup_cuda_ubuntu.sh` - creates an Ubuntu 22.04 Multipass container named `ubuntu-cuda-dev`

These scripts install the CUDA toolkit, development tools, and build dependencies in an isolated container environment.

---

## 6. Test the endpoint

```bash
curl -X POST http://localhost:${IMAGE_GEN_PORT}/generate \
     -H "Content-Type: application/json" \
     -d '{"prompt":"write code to train MNIST in pytorch","steps":256}'
```

You should get a JSON payload:

```json
{ "image_base64": "iVBORw0KGgoAAAANSUhEUgAA..." }
```

Decode it with `echo <b64> | base64 -d > out.png` to see the generated image.

---

## 7. Wire into Hermes

```bash
hermes tools --custom-image-gen http://localhost:${IMAGE_GEN_PORT}/generate
```

Hermes can now forward any prompt that requires image generation to this endpoint.

---

## 8. Notes & Troubleshooting

* **Crow installation** – If you cannot find `libcrow-dev`, build Crow from source and install the headers to `/usr/include/crow`.
* **GPU acceleration** – The Docker image is built with the NVIDIA base image and the pod is started with `--runtime nvidia`.  This gives the `llama-diffusion-cli` binary access to CUDA.  If you only have a CPU, omit the `--runtime nvidia` flag.
* **Model file** – `llama-diffusion-cli` expects a `*.gguf` model.  If you need to use a different one, replace `-m dream7b.gguf` in `run_cli`. Or when using stable-diffusion.cpp possible source here: https://huggingface.co/black-forest-labs/FLUX.1-dev/tree/main; Download them into their own sub-directory(e.g. models/); More likely done in CPU&RAM.
* **Performance** – The Crow server is lightweight; the bottleneck remains the diffusion inference performed by `llama-diffusion-cli`.  For higher throughput consider running multiple replicas or using a GPU‑dedicated server.

---

## 9. Next steps

1. A tiny, self‑contained HTTP **registry** for LAN‑based services advertising tooling capabilities. 
   Multiple tools are going to be available, some doing the same job(testing resource requirements and results, or scaling)
2. **Add authentication** – protect the `/generate` endpoint if you expose it externally.
3. **Expose a `/health` endpoint** – useful for Kubernetes liveness/readiness checks.
4. **Automate model download** – add a `RUN wget …` line in the Dockerfile if you want the image to be self‑contained.

---

*End of plan.*

---

## 10. Typical protocols for AI inference endpoints

| Protocol | How it is used with LLM / vision / speech APIs | When it is the best fit |
| --- | --- | --- |
| HTTPS (HTTP/1.1 or HTTP/2) | Standard REST-style calls such as `POST /v1/completions` (OpenAI, Anthropic, Azure OpenAI, etc.). Most providers expose a simple request-response JSON API over TLS. | • One-shot or batch queries where the whole prompt and response fit in a single request. <br> • Environments needing maximum compatibility with firewalls, CDNs, proxies, and serverless platforms. <br> • Situations where you want easy caching, retry logic, and stateless scaling. |
| HTTP/2 streaming (SSE / chunked transfer) | Same HTTPS endpoint, but the server keeps the connection open and streams partial token data (for example, `text/event-stream`). This enables progressive UI updates while staying on the familiar HTTP stack. | • Token-by-token streaming when you want low latency without introducing a separate protocol. <br> • Clients that cannot open WebSockets, such as some mobile web browsers with restrictive CSP. |
| WebSockets | After an initial HTTP upgrade, a persistent bidirectional channel carries JSON or binary messages such as prompts, partial responses, tool-call data, and client-side steering. Common in custom “agentic” UI layers and collaborative AI chatrooms. | • Real-time, bidirectional interactions with continuous generation, tool calls, and user interruptions. <br> • Multi-device session continuity where the client may send steering commands while the model streams tokens. <br> • Scenarios needing low-latency push notifications beyond simple token streams. |
| gRPC (HTTP/2-based) | Proto-defined services such as `InvokeModel` or `StreamResponse` with built-in client-side and server-side streaming. Used in internal microservice pipelines or high-throughput inference clusters. | • Service-to-service communication inside a data center where schema safety, multiplexing, and binary efficiency matter. <br> • Large-scale deployments requiring protobuf contracts and automatic code generation. |

## 11. Concise comparison: Boost.Beast vs CrowCpp vs nghttp2

| Library | HTTPS | HTTP/2 | WebSockets | Best for | Tradeoff |
| --- | --- | --- | --- | --- | --- |
| Boost.Beast | Yes, with OpenSSL | Limited / not first-class HTTP/2 | Yes | Custom proxies, clients, HTTP/1.1 services, WS upgrades | More low-level; significant boilerplate |
| CrowCpp | Yes, with TLS support | No native HTTP/2 support | Limited / not a primary focus | Fast small web APIs, local services, simple routing | Not ideal for streaming-heavy AI gateway |
| nghttp2 | N/A as a full app server; HTTP/2 engine | Yes, strong HTTP/2 support | Not a general web server | HTTP/2 transport, multiplexing, streaming | You still need app/server glue around it |

## 12. Development Environment Setup

This project provides scripts and documentation for setting up the development environment across different Linux distributions and container platforms.

- **Arch Linux**: Refer to `doc/README_arch.md` and `doc/setup_cuda_arch.sh`
- **Fedora**: Refer to `doc/README_fedora.md` and `doc/setup_cuda_fedora.sh`
- **Ubuntu**: Refer to `doc/README_ubuntu.md` and `doc/setup_cuda_ubuntu.sh`

Each script installs the CUDA toolkit, development dependencies, and configures the environment for building and running the Hermes tools gateway.

