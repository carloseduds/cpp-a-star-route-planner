IMAGE_NAME ?= cpp-a-star-route-planner

.PHONY: build run test clean

build:
	IMAGE_NAME=$(IMAGE_NAME) ./scripts/build.sh

run:
	IMAGE_NAME=$(IMAGE_NAME) ./scripts/run.sh

test:
	docker run --rm $(IMAGE_NAME) --help >/dev/null || true
	@echo "Unit tests run inside the build stage (optional). For native tests:"
	@echo "  cmake -S . -B build && cmake --build build && ctest --test-dir build"

clean:
	rm -rf build out
