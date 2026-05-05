# tiny-tiger

<img width="273" height="299" alt="image" src="https://github.com/user-attachments/assets/f6c15892-071d-427d-ac99-e0a63119cdd0" />

A tiny 2D game engine written in C++, compiled to WebAssembly via [Emscripten](https://emscripten.org), and usable directly from JavaScript in the browser.

Built as a portfolio/learning project with GitHub Copilot assistance. The goal is clean, readable C++ — not overly complex, but not sloppy either.

---

## What it does

The engine runs all game logic in C++ (compiled to WASM) and lets JavaScript handle rendering via the HTML5 Canvas API. This mirrors how real-world engines like Figma's core work.

**C++ / WASM side:**
- `Color` — RGBA color value
- `Vector2` — 2D vector math (add, subtract, scale, length)
- `Renderer` — Draws shapes, text, and paths to the HTML5 Canvas
- `KeyboardInput` — Tracks held/pressed/released keys each frame
- `MouseInput` — Canvas-relative cursor position and mouse button state
- `Engine` — Owns all subsystems, runs the game loop, accepts JS update and draw callbacks

**JavaScript side:**
- Loads the compiled `.wasm` module via `Module.onRuntimeInitialized`
- Provides update and draw callbacks to the engine
- Reads input through `KeyboardInput` and `MouseInput`

---

## Tech stack

| Tool | Purpose |
|---|---|
| C++17 | Core engine logic |
| Emscripten | Compiles C++ → WebAssembly |
| make | Build system |
| HTML5 Canvas | Rendering |

---

## Project structure

```
tiny-tiger/
├── src/
│   ├── engine.h        — class declarations (Color, Vector2, Renderer, KeyboardInput, MouseInput, Engine)
│   ├── engine.cpp      — implementation (EM_JS canvas calls, Emscripten HTML5 input callbacks)
│   └── bindings.cpp    — EMSCRIPTEN_BINDINGS block that exposes classes to JavaScript
├── index.html          — browser demo
└── Makefile            — build rules (requires emcc)
```

---

## Getting started

Install Emscripten first: https://emscripten.org/docs/getting_started/downloads.html

Emscripten Setup
Emscripten is installed via emsdk but not added to system/user environment variables permanently, to avoid conflicts with NVM and system Python.
Instead, activate it on demand per terminal session.
Activation
Add the following to your PowerShell profile (notepad $PROFILE):
```
powershellfunction Invoke-BatchFile {
    param([string]$Path)
    $tempFile = [IO.Path]::GetTempFileName()
    cmd /c "`"$Path`" && set > `"$tempFile`""
    Get-Content $tempFile | ForEach-Object {
        if ($_ -match "^([^=]+)=(.*)$") {
            [System.Environment]::SetEnvironmentVariable($matches[1], $matches[2])
        }
    }
    Remove-Item $tempFile
}

function Enable-Emscripten {
    Invoke-BatchFile "E:\Code\emsdk\emsdk_env.bat"
    Write-Host "Emscripten activated." -ForegroundColor Green
}
```
Then run this in any terminal session where you need Emscripten:
`powershellEnable-Emscripten`
The environment (emcc, em++, etc.) is active for that session only and resets when the terminal is closed.

After that...

```bash
# Build the WASM module
make

# Serve the project (required — browsers block WASM from file:// URLs)
python3 -m http.server 8080
```

Open [http://localhost:8080](http://localhost:8080) in your browser.

---

## Demo

The included `index.html` is an interactive demo:

- **Arrow Keys** or **WASD** — move the red player square
- **Left Click** on the canvas — move the green target circle
- **Left Mouse Button** (hold) — turns the player orange
- **Space** (hold) — makes the player pulse

---

## JavaScript API

### `Module.Color(red, green, blue [, alpha])`

Represents an RGBA color. `alpha` defaults to `255` (fully opaque).

```js
const red   = new Module.Color(255, 0, 0);
const glass = new Module.Color(0, 128, 255, 128);
```

### `Module.Vector2(x, y)`

A 2D vector with arithmetic helpers.

```js
const position = new Module.Vector2(100, 200);
const velocity = new Module.Vector2(50, 0);
const nextPosition = position.add(velocity.scale(deltaTimeInSeconds));
console.log(nextPosition.xPosition, nextPosition.yPosition);
console.log(nextPosition.getLength());
```

### `Module.Engine(canvasWidth, canvasHeight)`

The main engine object. Creates and sizes a `<canvas id="gameCanvas">` element.

```js
Module.onRuntimeInitialized = function () {
    const engine = new Module.Engine(800, 600);

    engine.setUpdateCallback(function (deltaTimeInSeconds) {
        // update game logic here
    });

    engine.setDrawCallback(function () {
        // draw the frame here
    });

    engine.run();
};
```

`engine.deltaTimeSinceLastFrame` — time in seconds between the last two frames (read inside your callbacks).

`engine.stop()` — cancels the game loop.

### `Renderer` — obtained via `engine.getRenderer()`

| Method | Description |
|--------|-------------|
| `clearScreen(color)` | Fill the whole canvas with a color |
| `drawFilledRectangle(x, y, width, height, color)` | Filled rectangle |
| `drawOutlinedRectangle(x, y, width, height, color, thickness)` | Rectangle outline |
| `drawFilledCircle(cx, cy, radius, color)` | Filled circle |
| `drawOutlinedCircle(cx, cy, radius, color, thickness)` | Circle outline |
| `drawLine(x1, y1, x2, y2, color, thickness)` | Line segment |
| `drawTextString(text, x, y, fontSize, color)` | Text using `sans-serif` |
| `beginPathDrawing()` | Start a custom path |
| `movePathTo(x, y)` | Move path cursor without drawing |
| `drawLineTo(x, y)` | Extend path with a line |
| `strokeCurrentPath(color, thickness)` | Stroke the current path |
| `fillCurrentPath(color)` | Fill the current path |
| `saveDrawingState()` | Push canvas state |
| `restoreDrawingState()` | Pop canvas state |
| `applyTranslation(x, y)` | Translate the canvas transform |
| `applyRotation(angleInRadians)` | Rotate the canvas transform |
| `applyScaling(xScale, yScale)` | Scale the canvas transform |
| `setGlobalAlphaValue(alpha)` | Set global opacity (0.0 – 1.0) |

### `KeyboardInput` — obtained via `engine.getKeyboardInput()`

```js
const keyboard = engine.getKeyboardInput();
keyboard.isKeyCurrentlyHeld('ArrowLeft')      // true while key is down
keyboard.wasKeyPressedThisFrame('Space')       // true on the first frame the key goes down
keyboard.wasKeyReleasedThisFrame('Enter')      // true on the frame the key is released
```

Key names match the standard `KeyboardEvent.key` values
(`'ArrowLeft'`, `'ArrowRight'`, `'ArrowUp'`, `'ArrowDown'`, `' '` for Space, `'a'`–`'z'`, etc.).

### `MouseInput` — obtained via `engine.getMouseInput()`

```js
const mouse = engine.getMouseInput();
mouse.getCursorXPosition()                      // cursor X relative to canvas
mouse.getCursorYPosition()                      // cursor Y relative to canvas
mouse.isMouseButtonHeld(0)                      // 0 = left, 1 = middle, 2 = right
mouse.wasMouseButtonPressedThisFrame(0)
mouse.wasMouseButtonReleasedThisFrame(0)
```

---

## Goals

- Learn C++ fundamentals through a real, usable project
- Understand how WebAssembly bridges C++ and JavaScript

---

## License

MIT
