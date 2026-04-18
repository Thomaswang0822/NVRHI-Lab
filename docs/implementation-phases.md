# Implementation Phases

This document outlines the incremental implementation path from a minimal triangle to a fully interactive graphics testbed.

**Current Status**: Phase 2 planning. Phase 1 complete (v0.1.6) - all three backends (D3D11, D3D12, Vulkan) working with visible triangle.

---

## Phase 1: NVRHI Foundation ✅ COMPLETE

**Goal**: Get NVRHI rendering a colored triangle on screen

**Key Tasks:**
1. Create Device Manager (`src/graphics/device_manager.h/cpp`)
   - Initialize NVRHI device for DX11/DX12/Vulkan
   - Create swap chain texture wrapper
   - Set up framebuffer
   - Uses IPlatformContext interface for backend abstraction

2. Create Basic Renderer (`src/graphics/basic_renderer.h/cpp`)
   - Simple graphics pipeline (vertex + pixel shaders)
   - Command list recording and execution
   - Present to screen

3. Integrate with wxWidgets
   - Get window handle from wxFrame
   - Pass to device manager
   - Render loop triggered by wxWidgets timer

4. Test across backends
   - Render a colored triangle
   - Verify each backend works (DX11/DX12/Vulkan)

**Success Criteria**: ✅ Colored triangle renders on screen with each backend (D3D11, D3D12, Vulkan)

**NVRHI Concepts Learned**:
- Device creation (D3D11, D3D12, Vulkan)
- Swap chain and backbuffer management
- Graphics pipeline state objects
- Command list recording and submission
- Resource state transitions (barriers)
- Frame synchronization

---

## Phase 2: Scene System & Objects

**Goal**: Render static 3D scene (background + 1 target)

**Key Tasks:**
1. Math Library Integration
   - Add GLM (cross-platform, header-only)
   - Helper functions: perspective(), lookAt()

2. Camera System (`src/graphics/camera.h/cpp`)
   - View/projection matrices
   - Constant buffer for camera data

3. Constant Buffer Infrastructure (`src/graphics/constant_buffer.h/cpp`)
   - Templated helper class for buffer management
   - Binding layout setup

4. Scene Object Base Class (`src/graphics/scene_object.h/cpp`)
   - Virtual `render()` method
   - Common resource management

5. Background Wall (`src/graphics/background_wall.h/cpp`)
   - Quad geometry with index buffer
   - Simple unlit shader

6. Simple Target (`src/graphics/target.h/cpp`)
   - Cube geometry
   - Basic lit shader

7. Scene Manager (`src/graphics/scene_manager.h/cpp`)
   - Orchestrates all scene objects
   - Manages shared resources (camera, depth buffer)

**Success Criteria**: Static 3D scene with background wall and one target cube

**NVRHI Concepts Learned**:
- Constant buffers (常量缓冲区)
- Index buffers (索引缓冲区)
- Resource bindings and binding layouts
- Textures and samplers (optional)

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
