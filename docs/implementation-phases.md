# Implementation Phases

This document outlines the incremental implementation path from a minimal triangle to a fully interactive graphics testbed.

## Phase 1: NVRHI Foundation

**Goal**: Get NVRHI rendering a colored triangle on screen

**Key Tasks:**
1. Create Device Manager (`src/graphics/device_manager.h/cpp`)
   - Initialize NVRHI device for DX11/DX12/Vulkan
   - Create swap chain texture wrapper
   - Set up framebuffer

2. Create Basic Renderer (`src/graphics/graphics_renderer.h/cpp`)
   - Simple graphics pipeline (vertex + pixel shaders)
   - Command list recording and execution
   - Present to screen

3. Integrate with wxWidgets
   - Get window handle from wxFrame
   - Pass to device manager
   - Render loop triggered by wxWidgets timer

4. Test across backends
   - Render a red triangle
   - Verify each backend works (DX11/DX12/Vulkan)

**Success Criteria**: Colored triangle renders on screen with each backend

**NVRHI Concepts Learned**:
- Device creation (d3d11, d3d12, vulkan)
- Swap chain texture wrapper
- Framebuffer setup
- Graphics pipeline state objects
- Command list recording
- Presenting

---

## Phase 2: Scene System & Objects

**Goal**: Render static 3D scene (background + 1 target)

**Key Tasks:**
1. Scene Object Interface (`src/graphics/scene_object.h/cpp`)
   - Base class for renderable objects
   - Virtual `render()` method
   - Owns resources (buffers, textures, pipeline)

2. Background Wall (`src/graphics/background_wall.h/cpp`)
   - Simple quad geometry
   - Different texture types for experiments (mipmaps, formats, etc.)

3. Simple Target (`src/graphics/target.h/cpp`)
   - Sphere or box geometry
   - Basic material

4. Camera System (`src/graphics/camera.h/cpp`)
   - View/projection matrices
   - Simple orbit or static view

**Success Criteria**: Static 3D scene with background wall and one target

**NVRHI Concepts Learned**:
- Buffers (vertex, index, constant)
- Textures (different formats)
- Resource bindings
- Descriptor sets
- Input layouts

---

## Phase 3: Interactive Scene

**Goal**: Clickable targets with basic feedback

**Key Tasks:**
1. Target Spawning System (`src/graphics/target_manager.h/cpp`)
   - Spawn targets at random positions
   - Manage target lifecycle (spawn → hit → despawn)

2. Mouse Hit Detection
   - Convert mouse click to ray in 3D space
   - Simple sphere/ray intersection
   - Trigger "hit" state

3. Visual Feedback
   - Change target color/material on hit
   - Disappear after delay

**Success Criteria**: Clickable targets, hit detection works, visual feedback displayed

**NVRHI Concepts Learned**:
- Dynamic resource updates
- Buffer mapping (for constant buffer updates)
- Resource state changes

---

## Phase 4: Test Scenarios

**Goal**: Menu-driven exploration of NVRHI features

**Key Tasks:**
1. Add wxWidgets menu items to switch scenarios
2. Texture Test Scenario
   - Different texture formats (RGBA8, RGBA16F, etc.)
   - Mipmap levels
   - Texture filtering modes

3. Primitive Type Scenario
   - Test all NVRHI primitive types
   - Triangle list, strip, patch list, etc.

4. Pipeline State Scenario
   - Different rasterizer states
   - Blend modes
   - Depth/stencil configurations

Each scenario creates appropriate scene objects and lets user experiment.

**Success Criteria**: Menu can switch between texture/primitive/pipeline tests

**NVRHI Concepts Learned**:
- Pipeline state variations
- Resource state transitions
- Different resource formats

---

## Phase 5: Advanced Features

**Goal**: Explore advanced NVRHI APIs

**Key Tasks:**
1. Ray Tracing Scenario
   - BLAS/TLAS setup
   - Simple ray tracing pipeline
   - Render targets using ray tracing

2. Compute Shaders
   - Post-processing effects
   - Target animation

3. Multiple Queues
   - Parallel command list recording
   - Multi-queue rendering

**Success Criteria**: Advanced features (ray tracing, compute) work as expected

**NVRHI Concepts Learned**:
- Ray tracing (BLAS/TLAS, ray tracing pipeline)
- Compute shaders
- Advanced execution patterns (parallel recording, multi-queue)

---

## Notes

- **Progression**: Each phase builds on the previous one
- **Testing**: Test each backend (DX11/DX12/Vulkan) after phase completion
- **Validation**: NVRHI validation layer enabled throughout for debugging
- **Backend Switching**: App restart required to switch backends (no runtime switching initially)
