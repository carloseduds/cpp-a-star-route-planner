#!/usr/bin/env bash
set -euo pipefail

# Fix for Git Bash/MSYS on Windows: avoid path mangling
if [[ "${OSTYPE:-}" == msys* || "${OSTYPE:-}" == cygwin* || "${OSTYPE:-}" == win32* ]]; then
  export MSYS_NO_PATHCONV=1
  export MSYS2_ARG_CONV_EXCL="*"
fi

IMAGE_NAME="${IMAGE_NAME:-cpp-a-star-route-planner}"
OUT_DIR="${OUT_DIR:-$(pwd)/out}"
mkdir -p "${OUT_DIR}"

ARGS=("$@")

has_flag () {
  local flag="$1"
  for a in "${ARGS[@]:-}"; do
    if [[ "$a" == "$flag" ]]; then
      return 0
    fi
  done
  return 1
}

# Always ensure an OSM input inside container
if ! has_flag "-f"; then
  ARGS=(-f /app/map.osm "${ARGS[@]}")
fi

# Always ensure output goes to the bind-mounted folder
if ! has_flag "--out"; then
  ARGS=(--out /out/map_routed.png "${ARGS[@]}")
fi

# If user provided no coords, use defaults
if ! has_flag "--start"; then
  ARGS=(--start 20 30 "${ARGS[@]}")
fi
if ! has_flag "--end"; then
  ARGS=(--end 50 40 "${ARGS[@]}")
fi

echo "==> Running container (output will be written to: ${OUT_DIR})"
docker run --rm \
  -v "${OUT_DIR}:/out" \
  "${IMAGE_NAME}" \
  "${ARGS[@]}"

echo "==> Expected artifact: ${OUT_DIR}/map_routed.png"
ls -lah "${OUT_DIR}/map_routed.png" 2>/dev/null || {
  echo "!! map_routed.png not found in ${OUT_DIR}"
  echo "   Tip: check if your program printed 'Wrote: /out/map_routed.png'"
  exit 1
}

echo "==> Done."
