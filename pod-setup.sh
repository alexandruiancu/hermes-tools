#!/usr/bin/env bash
set -euo pipefail

source .env
IMAGE_GEN_PORT=${IMAGE_GEN_PORT:-8000}

# Build image‑gen only
podman build -t image-gen:latest ./image-gen

POD_NAME=hermes-tools
podman pod rm -f "$POD_NAME" || true
podman pod create \
    --name "$POD_NAME" \
    --runtime nvidia \
    --publish ${IMAGE_GEN_PORT}:8000 \
    --infra=true

# Run image‑generation container inside the pod
# Default to Crow mode; change to "fcgi" if desired
podman run \
    --pod "$POD_NAME" \
    --name image-gen \
    image-gen:latest crow

echo "Pod \"$POD_NAME\" is running.  Image‑gen on http://localhost:${IMAGE_GEN_PORT}"
