EMSCRIPTEN_CC = emcc
SOURCE_FILES = src/engine.cpp src/bindings.cpp
OUTPUT_FILE = dist/tiny_tiger.js

COMPILE_FLAGS = -std=c++17 -O0
LINKER_FLAGS = --bind -s WASM=1 -s ALLOW_MEMORY_GROWTH=1

all: $(OUTPUT_FILE)

$(OUTPUT_FILE): $(SOURCE_FILES) src/engine.h
	mkdir -p dist
	$(EMSCRIPTEN_CC) $(COMPILE_FLAGS) $(SOURCE_FILES) $(LINKER_FLAGS) -o $(OUTPUT_FILE)

clean:
	rm -rf dist

.PHONY: all clean
