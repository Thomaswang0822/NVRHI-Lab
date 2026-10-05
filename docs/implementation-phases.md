# Implementation Phases

This document outlines the incremental implementation path from a minimal triangle to a fully interactive graphics testbed.

**Current Status**: Phase 2 in progress (v0.1.9). Steps 1–4 complete (math, camera + fly controls, constant buffers, scene manager system); window resize handling remaining.

---

## Verification Rule

Every feature/component must ship with an **acceptance check** — a simple way to observe that it actually works. A task without a check is not planned; a feature without a check is not done.

Graphics code fails *silently* (no crash, just a wrong image), so "compiles and runs" is not verification. Tiers, cheapest that reveals the likely errors:

1. **Interactive control** — mouse/keyboard drives the feature (most intuitive, highest cost)
2. **Scripted animation** — e.g. an orbiting camera; wrong matrices become visible within seconds (best cost/benefit)
3. **Automated check** — `--frameNum=N` smoke run, logged sanity values, screenshot comparison (regression-friendly)

Verify at the moment the feature lands: each new layer built on an unverified layer multiplies debugging cost later.

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

1. Math Library Integration ✅
   - Add GLM (cross-platform, header-only)
   - Helper functions: perspective(), lookAt()
   - **Acceptance check** (covered by 2.2's orbit): GLM config errors (handedness, depth range) would show up immediately as a broken view

2. Camera System ✅ (`src/graphics/camera.h/cpp`)
   - View/projection matrices
   - Constant buffer for camera data
   - **Acceptance check** ✅ implemented: interactive **god-mode fly camera** (tier 1, `src/graphics/camera_controller.h/cpp`) — WASD/RF six-axis movement (strafe horizontal, up/down world-vertical), arrow keys look (yaw/pitch), Q/E in-place turning; the camera is fully self-centered with zero coupling to scene objects (objects may move/rotate out of view); per-backend Y-flip normalized so D3D/Vulkan render identically; init-time NDC sanity log (tier 3). Previously verified via scripted orbit (tier 2), which caught NVRHI's default back-face culling hiding the triangle half the orbit
   - **Note**: For Vulkan, must set `VulkanBindingOffsets` to 0 to match DXC's register-to-binding mapping

3. Constant Buffer Infrastructure ✅ (`src/graphics/constant_buffer.h`)
   - Templated helper class for buffer management
   - Binding layout setup
   - **Acceptance check** (covered by 2.2's orbit): camera constants are re-uploaded every frame; a stale or broken buffer freezes/corrupts the orbiting view

4. Scene Manager System ✅ (`src/graphics/scene_manager.h/cpp`)
   - Classic renderer shape: **scene = loader + storage** — no per-object-type classes, no per-object shaders
   - Generic `Mesh` (VB/IB/count) + `Material` (pure data: base color, lit/unlit, grid) + `SceneObject` POD (mesh index, material, transform); geometry from `geometry.h/cpp` (`MakeGridWall`/`MakeCube` today, file parser later — same interface)
   - ONE pipeline and ONE VS/PS pair for the whole scene; per-draw `ObjectConstants` (ModelMatrix + BaseColor + lighting/grid flags) at b1 beside camera b0; back-face culling deliberate (wrong winding = missing face, tier-1 visible)
   - Frontend: grid wall + lit cube; test triangle deleted
   - **Acceptance check** ✅ implemented (tier 1): fly the god camera — grid wall behind lit cube, depth ordering and culling correct from every angle; tier-3 `--frameNum=300` Vulkan run clean

5. Window Resize Handling
   - Swapchain recreation + depth buffer resize + camera aspect ratio update (no resize path exists today; version-log 0.1.5 claim was stale)
   - **Acceptance check**: drag the window edge — image follows without distortion or crash (tier 1)

**Success Criteria**: Static 3D scene with background wall and one target cube

**NVRHI Concepts Learned**:

- Constant buffers (常量缓冲区): camera (b0, shared) + per-object ModelMatrix (b1)
- Index buffers (索引缓冲区) and drawIndexed
- Resource bindings and binding layouts (multi-slot binding sets)
- Deliberate back-face culling and winding order

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
