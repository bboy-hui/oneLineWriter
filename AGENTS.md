# AGENTS.md

## Build

```bash
cmake -S . -B build
cmake --build build --config Release
```

## Run

```bash
./build/apps/desktop/Release/owl_desktop.exe --backend=vulkan
./build/apps/desktop/Release/owl_desktop.exe --backend=opengl
```

Vulkan is the default backend when no `--backend` flag is specified.

## Debug (VS Code)

Use preset `123-debug` which builds to `out/build/123/apps/desktop/Debug/owl_desktop.exe`.

## Architecture

- `apps/desktop` - ImGui entrypoint
- `src/core` - lifecycle, config, logging, task orchestration
- `src/gcode` - path data,工艺 parameters, GCode generation
- `src/device` - serial/GRBL communication, send queue, state machine
- `src/render` - scene abstraction with `IRenderBackend` interface

## 3D Viewport Rendering

Two viewport windows exist in `apps/desktop/src/`:
- `Viewport3DWindow.cpp` - OpenGL native 3D rendering in child window
- `Viewport3DWindowVulkan.cpp` - Vulkan path (currently ImGui draw-list based projection)

The `--backend` flag selects which viewport panel is shown. The ImGui UI itself always uses OpenGL.

## Build Options

- `OWL_ENABLE_VULKAN_BACKEND` (default ON)
- `OWL_ENABLE_OPENGL_BACKEND` (default ON)
- C++20 required; MSVC uses `/W4 /permissive-`

## Notes

- No test framework or linting config detected in this repo
- `3rd/` contains spdlog and imgui as git submodules/externals
