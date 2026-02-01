# Implementation Plan

## Project Structure

```
NVRHI-Lab/
├── src/              - All source code
│   ├── main.cpp      - Application entry point
│   └── [components]/ - Feature modules (textures/, common/, etc.)
├── build/            - CMake working directory
├── bin/              - Executables and runtime resources
├── docs/             - Documentation
├── thirdparty/       - Third-party libraries (NVRHI)
└── .opencode/        - AI agent context and skills
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

**Tasks:**
- [ ] Create Device Manager (`src/graphics/device_manager.h`) - Initialize NVRHI device for DX11/DX12/Vulkan, create swap chain texture wrapper, set up framebuffer
- [ ] Create Basic Renderer (`src/graphics/graphics_renderer.h`) - Simple graphics pipeline (vertex + pixel shaders), command list recording and execution, present to screen
- [ ] Integrate with wxWidgets - Get window handle from wxFrame, pass to device manager, render loop triggered by wxWidgets timer
- [ ] Test: Render a red triangle → Verify all backends work (DX11/DX12/Vulkan via app relaunch)

**NVRHI Concepts**: Device creation, swap chain, pipelines, command lists

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
