# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build

```bash
cmake -S . -B build
cmake --build build --config Release
```

Run with Visual Studio presets (from project root):
```bash
cmake --preset=MSVC
cmake --build --preset=msvc1-debug
```

## Architecture

### Modules

- `apps/desktop`：Desktop application entry point (ImGui UI)
- `src/core`：Application lifecycle, configuration, logging, task orchestration
- `src/render`：Rendering abstraction with `IRenderBackend` interface; supports Vulkan and OpenGL backends
- `src/gcode`：Path data, process parameters, GCode generation
- `src/device`：Serial communication, GRBL state machine, send throttling and acknowledgment handling
- `src/image`：Image processing utilities
- `3rd/`：Third-party dependencies (imgui, spdlog, OpenCV)

### Rendering Abstraction

- Core interface: `IRenderBackend`
- Unified input: `CameraState`, `Polyline3D`
- Backend types: `BackendType::Vulkan`, `BackendType::OpenGL`
- Business logic must not depend on specific graphics APIs

### Dependencies

- OpenCV libraries pre-built in `install/` directory (DLLs)
- Fonts in `install/` for text rendering
- imgui and spdlog in `3rd/`

## Development Notes

- Many source files (core, device, gcode) are currently deleted (marked D in git status) - the project is in a refactoring state
- Current `apps/main.cpp` is a minimal stub printing "hello world"
- Target executable: `build/apps/desktop/Release/owl_desktop.exe` (or equivalent in out/build/)
