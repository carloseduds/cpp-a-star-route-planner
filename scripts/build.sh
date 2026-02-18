#!/usr/bin/env bash
set -euo pipefail

IMAGE_NAME="${IMAGE_NAME:-cpp-a-star-route-planner}"
DOCKERFILE="${DOCKERFILE:-Dockerfile}"

echo "==> Building Docker image: ${IMAGE_NAME}"
docker build -t "${IMAGE_NAME}" -f "${DOCKERFILE}" .
echo "==> Done."
