# tiny-tiger

# tiny-tiger

A tiny 2D game engine written in C++, compiled to WebAssembly via Emscripten, and usable directly from JavaScript in the browser.

Built as a portfolio/learning project with GitHub Copilot assistance. The goal is clean, readable C++ — not overly complex, but not sloppy either.

---

## What it does

The engine runs all game logic in C++ (compiled to WASM) and lets JavaScript handle rendering via the HTML5 Canvas API. This mirrors how real-world engines like Figma's core work.

**C++ / WASM side:**
- `Vec2` — 2D vector math (add, subtract, scale, dot product, normalize, lerp, distance)
- `Entity` — A game object with position, velocity, and size
- `World` — Holds all entities and runs the update loop
- `Physics` — Applies velocity, gravity, and AABB collision detection
- `Bindings` — Exposes the C++ API to JavaScript via Emscripten

**JavaScript side:**
- Loads the compiled `.wasm` module
- Runs a `requestAnimationFrame` game loop
- Reads entity state from WASM and draws to Canvas
- Handles keyboard/mouse input and passes it into C++

---

## Tech stack

| Tool | Purpose |
|---|---|
| C++17 | Core engine logic |
| Emscripten | Compiles C++ → WebAssembly |
| CMake | Build system |
| Bun | Frontend dev server |
| HTML5 Canvas | Rendering |

---

## Project structure

```
tiny-tiger/
├── src/
│   ├── vec2.h / vec2.cpp        # 2D vector math
│   ├── entity.h / entity.cpp    # Game object
│   ├── world.h / world.cpp      # Entity manager + update loop
│   ├── physics.h / physics.cpp  # Velocity, gravity, collision
│   └── bindings.cpp             # Emscripten JS bindings
├── web/
│   ├── index.html               # Entry point
│   ├── main.js                  # Game loop + Canvas renderer
│   └── input.js                 # Keyboard/mouse input
├── CMakeLists.txt
└── README.md
```

---

## Build phases

- [x] Phase 1 — Environment setup (Emscripten + CMake + Bun)
- [ ] Phase 2 — Vec2 math library
- [ ] Phase 3 — Entity & World classes
- [ ] Phase 4 — Physics (velocity, gravity, AABB collision)
- [ ] Phase 5 — Emscripten bindings (C++ → JS)
- [ ] Phase 6 — JavaScript frontend (Canvas renderer + input)
- [ ] Phase 7 — Demo game

---

## Getting started

> Full setup instructions coming soon. Requirements: Emscripten (`emsdk`), CMake, Node.js.

```bash
# Clone the repo
git clone https://github.com/YOUR_USERNAME/cppge.git
cd cppge

# Build the WASM module
emcmake cmake -B build
cmake --build build

# Start the frontend
cd web
bun add
bun run dev
```

---

## Goals

- Learn C++ fundamentals through a real, usable project
- Understand how WebAssembly bridges C++ and JavaScript

---

## License

MIT
