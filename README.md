# cpp-a-star-route-planner (C++ / OpenStreetMap / A*)

A **route planner** in C++ that reads an **OpenStreetMap (.osm)** extract, converts the map into a **navigable graph**, and finds the best route using **A\***.  
The result is rendered as an image (`map_routed.png`) with the computed path overlaid.

> **Portfolio focus:** this repo is organized to be **easy to run** (Docker), have a **clear README**, and highlight key technical decisions (graph modeling, A\* heuristic, rendering pipeline).

---

## Demo (clone → 1 command → run)

### Option A — Docker (recommended)

```bash
./scripts/build.sh
./scripts/run.sh
````

At the end, you should see:

* `./out/map_routed.png` (generated artifact)

> Tip: by default it runs with example coordinates (`--start 20 30 --end 50 40`).
> You can pass other parameters:
>
> ```bash
> ./scripts/run.sh --start 80 50 --end 20 10
> ```

### Option B — docker compose (if you prefer)

```bash
docker compose up --build
```

---

## Expected output

The final image is saved to `out/map_routed.png` (via Docker bind mount).

Example (already included in the repo):

![Route example](instruction_images/map_routed.png)

---

## How it works (architecture)

High-level pipeline:

1. **OSM parsing**: read the OpenStreetMap XML (`map.osm`) and extract nodes, ways, and relevant relations.
2. **Graph construction**: connect adjacent nodes with edge costs based on distance.
3. **A***: compute the shortest path between the start and end points.
4. **Render**: draw the map (roads, water, buildings, etc.) + overlay the computed route.
5. **PNG export**: generate `map_routed.png` (headless by default).

---

## A* (the essentials)

* A* combines:

  * **cost so far** to reach the current node (g)
  * **heuristic** cost to reach the goal (h)
* Here, the heuristic is the **Euclidean distance** (admissible), which helps reduce the search space while preserving optimality (for the chosen model).
* Typical trade-offs:

  * “stronger” heuristics → faster search, but must remain admissible
  * very dense graphs → more memory/time; simplified graphs → may lose fidelity

---

## Tech stack

* **C++17**
* **CMake**
* **OpenStreetMap** (`.osm` input)
* **pugixml** (XML parsing — vendored in `thirdparty/`)
* **googletest** (unit tests — vendored in `thirdparty/`)
* **cpp-io2d (P0267 RefImpl)** for rendering (fetched automatically via `FetchContent`)

> Note: rendering depends on system libraries (e.g., Cairo). Docker takes care of these dependencies.

---

## Running with parameters

The binary accepts:

* `-f <file.osm>`: path to the OSM file
* `--out <file.png>`: output PNG path
* `--start x y` and `--end x y` (0..100): normalized coordinates
* `--show`: opens an interactive window (requires a graphical environment).
  **By default, it runs headless and produces a PNG.**

Example using the Docker script:

```bash
./scripts/run.sh -f /app/map.osm --out /out/map_routed.png --start 5 10 --end 80 90
```

---

## Native build (without Docker) — optional

> If you want to compile on your host, you’ll need build dependencies (CMake, a compiler, Cairo and related libs).

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/OSM_A_star_search -f map.osm --out map_routed.png --start 20 30 --end 50 40
```

---

## Tests

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build
```

---

## What I learned / technical decisions (short and honest)

* How to **model a real-world map (OSM)** as a graph while keeping search cost under control.
* How to apply **A*** with a consistent heuristic and evaluate its impact on search time.
* How to make a C++ project **portfolio-ready**: reproducible dependencies (Docker), copy/paste commands, and value-focused documentation.
* How to separate responsibilities: **model (OSM/graph)**, **planner (A*)**, and **rendering**.

---

## Troubleshooting

### 1) `std::system_error: Input/output error` when running

This usually happens when the program tries to open a window in a headless environment (VM, CI, WSL, Udacity workspace).
**Fix:** use headless mode (default) and **don’t** pass `--show`.

### 2) Build fails due to missing graphics libraries on the host

Use Docker (`./scripts/build.sh`) — it installs dependencies and builds inside the container.

### 3) “.osm file not found”

Make sure you are pointing to a valid path with `-f`. In Docker, the example file is at `/app/map.osm`.

### 4) PNG not appearing on the host

The file is written to the mounted `./out` directory. Check `out/map_routed.png` after running the container.

---

## Repository structure

* `src/` — model, planner, and rendering implementation
* `test/` — unit tests (GoogleTest)
* `thirdparty/` — vendored dependencies (pugixml, googletest)
* `instruction_images/` — images used by the README
* `map.osm`, `map.png` — sample files for quick runs

---

## License

See [`LICENSE`](LICENSE).
