# Hermes Self‑Host Tools Plan

This plan explains how to expose the pre‑built **`llama-diffusion-cli`** binary behind an HTTP server using a **C++ Crow** wrapper, package it in a Docker image, and wire the endpoint into Hermes Agent.

---

## 1. Directory layout

```
~/services/
└─ hermes-tools/          ← Root folder for the Hermes tools
   ├─ image-gen/          ← Go implementation (go.mod, main.go, Dockerfile)
   │  ├─ Dockerfile
   │  ├─ go.mod
   │  └─ main.go
   ├─ tts/                ← Python implementation from A
   │  ├─ Dockerfile
   │  ├─ requirements.txt
   │  └─ app.py
   └─ pod-setup.sh
```

---

## 2. Crow server (`server.cpp`)

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
