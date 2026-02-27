.PHONY: build run test clean docker

build:
	cmake -B build -G Ninja && cmake --build build
	@mkdir -p build/fonts
	@cp -n fonts/Inter-Regular.ttf build/fonts/ 2>/dev/null || true

run: build
	./build/zoomfolder

test: build
	ctest --test-dir build --output-on-failure

clean:
	rm -rf build

docker:
	docker build --target export -o dist .
