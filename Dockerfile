# syntax=docker/dockerfile:1
FROM ubuntu:22.04 AS build

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    git \
    pkg-config \
    ca-certificates \
    libcairo2-dev \
    libpng-dev \
    libjpeg-dev \
    libtiff-dev \
    libx11-dev \
    libxext-dev \
    libxrandr-dev \
    libgraphicsmagick++-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace
COPY . .

RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
 && cmake --build build -j

# -----------------------------
# Runtime image
# -----------------------------
FROM ubuntu:22.04 AS runtime

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    libcairo2 \
    libpng16-16 \
    libjpeg-turbo8 \
    libtiff5 \
    libx11-6 \
    libxext6 \
    libxrandr2 \
    libgraphicsmagick-q16-3 \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY --from=build /workspace/build/OSM_A_star_search /app/OSM_A_star_search
COPY --from=build /workspace/map.osm /app/map.osm
COPY --from=build /workspace/map.png /app/map.png

ENV OUT_DIR=/out
RUN mkdir -p /out

ENTRYPOINT ["/app/OSM_A_star_search"]
CMD ["-f", "/app/map.osm", "--out", "/out/map_routed.png", "--start", "20", "30", "--end", "50", "40"]
