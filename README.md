# Hermes Self‑Host Tools Plan

This plan explains how to expose the pre‑built **`llama-diffusion-cli`** binary behind an HTTP server using a **C++ Crow** wrapper, package it in a Docker image, and wire the endpoint into Hermes Agent.

Different path alltogether:
https://github.com/huggingface/diffusers

---

## 1. Directory layout

```
~/services/hermes-tools/
├─ cmake        - CPM helpers
├─ pod-setup.sh - containerization to easy deploy
├─ .env         - runtime parametrization
├─ sd_server1/  - llama image generation A(probably obsolete or umaintained)
├─ sd_server2/  - leejet (most promising)
└─ sd_server3/  - tiny dream
   └─ models/
```
---

## 2. Crow server (`server.cpp`)
This component is implemented in the sub‑projects `sd_server1` and `sd_server3`. 
It serves as a lightweight HTTP wrapper around the underlying LLM inference binary, exposing an endpoint that mimics the OpenAI Chat Completion API. By forwarding requests to the model through Crow, downstream services can interact with the local LLM using familiar API calls without needing direct access to the binary.

---

## 3. CMake build file (`CMakeLists.txt`)

---

## 4. Dockerfile

> **Why two stages?** The build stage pulls the C++ compiler and Crow dev libs, compiles the server, then copies only the compiled binary and the `llama-diffusion-cli` binary into the minimal runtime image, keeping the final image size small.

---

## 5. Build & run instructions

> The container will start the Crow server listening on port 8000 inside the pod.

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

