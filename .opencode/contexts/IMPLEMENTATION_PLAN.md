# Implementation Plan

## Project Structure

```
NVRHI-Lab/
├── src/              - All source code
│   ├── shaders/      - HLSL shader files (.hlsl)
│   ├── graphics/     - Graphics components
│   │   ├── platform/ - Platform abstraction (d3d12_context, etc.)
│   │   ├── device_manager - NVRHI device and swap chain management
│   │   └── basic_renderer - Triangle rendering for Phase 1
│   ├── frontend/     - wxWidgets UI components
│   └── main.cpp      - Application entry point
├── cmake/           - CMake modules (shaders.cmake)
├── build/           - CMake working directory
├── bin/              - Executables and runtime resources (including shaders/)
├── docs/             - Documentation
├── thirdparty/       - Third-party libraries (NVRHI, wxWidgets)
└── .opencode/       - AI agent context and skills
```

## Learning Sequence
- [ ] Basic NVRHI concepts (resources, pipelines, bindings)
- [ ] Command lists and execution
- [ ] Resource management (textures, buffers)
- [ ] Pipeline states and shaders
- [ ] Advanced features (ray tracing, compute)

## Feature Modules
- [ ] common/ - Shared utilities, helpers
- [ ] textures/ - Texture (纹理) examples
- [ ] buffers/ - Buffer (缓冲区) examples
- [ ] pipelines/ - Pipeline state management
- [ ] bindings/ - Resource binding systems

## Implementation Phases

### Phase 1: NVRHI Foundation (Priority: HIGH)
**Goal**: Get NVRHI rendering a colored triangle on screen

**Status**: In Progress (D3D12 working, D3D11/Vulkan not implemented)

**Current Version**: 0.1.5

**Tasks:**
- [x] Create Device Manager (`src/graphics/device_manager`)
- [x] Create shader files (`src/shaders/triangle.vs.hlsl`, `src/shaders/triangle.ps.hlsl`)
- [x] Setup CMake shader compilation (`cmake/shaders.cmake`)
- [x] Create Basic Renderer (`src/graphics/basic_renderer`)
- [x] Integrate with wxWidgets render loop
- [x] Refactor to IPlatformContext (RAII-compliant)
- [x] Test: Colored triangle renders on D3D12
- [ ] Implement D3D11 backend
- [ ] Implement Vulkan backend

**NVRHI Concepts**: Device creation, swap chain, pipelines, command lists, resource barriers

### Phase 2: Scene System & Objects (Priority: HIGH)
**Goal**: Render static 3D scene (background + 1 target)

**Tasks:**
- [ ] Scene Object Interface (`src/graphics/scene_object.h`) - Base class for renderable objects, virtual `render()` method, owns resources (buffers, textures, pipeline)
- [ ] Background Wall (`src/graphics/background_wall.h`) - Simple quad geometry, different texture types for experiments
- [ ] Simple Target (`src/graphics/target.h`) - Sphere or box geometry, basic material
- [ ] Camera System (`src/graphics/camera.h`) - View/projection matrices, simple orbit or static view

**NVRHI Concepts**: Buffers, textures, resource bindings, descriptor sets

### Phase 3: Interactive Scene (Priority: MEDIUM)
**Goal**: Clickable targets with basic feedback

**Tasks:**
- [ ] Target Spawning System (`src/graphics/target_manager.h`) - Spawn targets at random positions, manage target lifecycle
- [ ] Mouse Hit Detection - Convert mouse click to ray in 3D space, simple sphere/ray intersection, trigger "hit" state
- [ ] Visual Feedback - Change target color/material on hit, disappear after delay

**NVRHI Concepts**: Dynamic resource updates, buffer mapping, state changes

### Phase 4: Test Scenarios (Priority: MEDIUM)
**Goal**: Menu-driven exploration of NVRHI features

**Tasks:**
- [ ] Add wxWidgets menu items to switch scenarios
- [ ] Texture Test Scenario - Different texture formats (RGBA8, RGBA16F), mipmap levels, texture filtering modes
- [ ] Primitive Type Scenario - Test all NVRHI primitive types (triangle list, strip, patch list, etc.)
- [ ] Pipeline State Scenario - Different rasterizer states, blend modes, depth/stencil configurations

**NVRHI Concepts**: Pipeline state variations, resource state transitions

### Phase 5: Advanced Features (Priority: LOW)
**Goal**: Explore advanced NVRHI APIs

**Tasks:**
- [ ] Ray Tracing Scenario - BLAS/TLAS setup, simple ray tracing pipeline, render targets using ray tracing
- [ ] Compute Shaders - Post-processing effects, target animation
- [ ] Multiple Queues - Parallel command list recording, multi-queue rendering

**NVRHI Concepts**: Ray tracing, compute, advanced execution patterns
