# Version Log

## 0.1.0

A minimal but runnable app with wxWidgets GUI.

- NVRHI has been added via CMake but not used in app.
- Design and implementation roadmap have been decided.

## 0.1.1

Phase 1, Task 1: Device Manager Implementation

- Created DeviceManager with multi-backend support (D3D11/D3D12/Vulkan stub)
- Added performance design documentation (docs/performance-design-choices.md)
- Documented architectural decision: Runtime switching vs polymorphism analysis
- Concluded: Architectural choice has negligible performance impact (<0.001%)

## 0.1.2

Phase 1: HLSL Shader Compilation Fix

- Fixed VS HLSL compilation issue: shaders were using default SM 4.0_9_3 instead of SM 6.4
- Solution: Configure VS's native HLSL compiler via CMake `set_source_files_properties()` with VS_SHADER_TYPE, VS_SHADER_MODEL (6.4), VS_SHADER_ENTRYPOINT, and VS_SHADER_OBJECT_FILE_NAME properties
- Removed DXC custom command approach - now uses VS native compiler
- Added basic_renderer.h header file stub for rendering pipeline

## 0.1.3

Phase 1: BasicRenderer Implementation

- Created BasicRenderer class (`src/graphics/basic_renderer.h/cpp`)
- Implemented vertex buffer creation with colored triangle vertices
- Created graphics pipeline with vertex and pixel shaders
- Added depth buffer (D32 format) for depth testing
- Implemented command list recording and execution
- Render loop: BeginFrame → Render → Present pattern established

## 0.1.4

Phase 1: Platform Abstraction Layer

- Created IPlatformContext interface (`src/graphics/platform/platform_context.h`)
- Implemented D3D12Context (`src/graphics/platform/d3d12_context.h/cpp`)
- Refactored DeviceManager to use IPlatformContext
- D3D12Context handles: DXGI factory, adapter selection, device creation, swap chain, command queue, fence synchronization
- DeviceManager now pure NVRHI API after initialization (no raw D3D12 pointers)
- RAII-compliant resource management throughout

## 0.1.5

Phase 1: wxWidgets Render Loop Integration

- Integrated BasicRenderer with wxWidgets timer-based render loop
- Added backend selection at startup (D3D12 works, D3D11/Vulkan stubs)
- Window resize handling implemented
- Frame timing and presentation working
- Fixed rendering bug: Added depth buffer attachment for proper depth testing
- **Colored triangle now visible on D3D12 backend**

## 0.1.6

Phase 1 Complete: Vulkan Backend Implementation

- Implemented VulkanContext class with full Vulkan device/swap chain management
- Added two-phase Vulkan dispatcher initialization for Vulkan-Hpp dynamic dispatch
- Enabled Vulkan 1.3 features (timelineSemaphore, dynamicRendering, synchronization2)
- Refactored IPlatformContext from pure interface to concrete base class with protected members
- Standardized swap chain format to RGBA8_UNORM across all backends
- Added GetSwapChainFormat() for consistent format access in renderer
- Fixed resource state tracking with keepInitialState flag for back buffers
- Added graphics-backends.md documentation comparing D3D11/D3D12/Vulkan APIs
- **All three backends (D3D11, D3D12, Vulkan) now working - Phase 1 complete!**

## 0.1.7

Phase 2: Camera System & Constant Buffer Infrastructure

- Added GLM math library as git submodule (thirdparty/glm)
- Created math_types.h with GLM configuration for left-handed coordinate system
- Implemented Camera class with view/projection matrix generation
- Created ConstantBuffer<T> templated helper class for GPU buffer management
- Added CameraConstants struct for CPU-GPU data sharing
- Created shader cbuffer headers (HLSL + GLSL) for camera uniforms
- Integrated camera with BasicRenderer using binding layout/set
- Updated vertex shader to use ViewProjectionMatrix transformation
- Added WIN32_LEAN_AND_MEAN with explanatory comment to avoid winsock conflicts
- Added --frameNum=N command line option for automated testing
- Fixed Vulkan binding offset mismatch: NVRHI defaults constantBuffer=256, DXC maps register(b0) to binding 0
- **All three backends (D3D11, D3D12, Vulkan) working with camera transformation**

## 0.1.8

Phase 2: Interactive God-Mode Fly Camera + Backend Y-Flip Normalization

- Added **Verification Rule** to implementation-phases.md: every task ships with an acceptance check (interactive / scripted animation / automated); no check = not done
- New `CameraController` (`src/graphics/camera_controller.h/cpp`): god-mode free-fly camera, fully self-centered with zero coupling to scene objects — **W/S/A/D** move along camera-local axes (strafe always horizontal), **R/F** world-vertical crane movement, **arrow keys** look (yaw/pitch, pitch clamped to ±89°), **Q/E** turn in place around the camera's own vertical axis (CCW/CW from above); diagonal movement normalized; control hint shown in the status bar. Serves as the camera system's tier-1 acceptance check: any matrix/attitude error is observable the moment a key is pressed
- Semantic `CameraInput` bitmask decouples the graphics layer from wxWidgets; the frontend polls physical key state each frame via `wxGetKeyState` (macOS: CGEventSourceKeyState / Windows: GetAsyncKeyState) — focus-independent; key bindings live in one table
- Normalized D3D vs Vulkan NDC Y direction: `Camera::SetFlipY()` negates projection Y for Vulkan (D3D +Y up, Vulkan +Y down) so both backends render identically; init-time NDC sanity log projects the triangle's top vertex through the VP matrix — (0, ±0.40, 0.97) matches hand calculation
- Fixed: NVRHI rasterizer defaults to back-face culling — now explicit `setCullNone()` until real geometry re-enables it deliberately (winding)
- Documented window resize handling as new Phase 2 task 5 (swapchain recreation + aspect update; no resize path exists today — 0.1.5 claim was stale)

## 0.1.9

Phase 2: Scene System (generic mesh/material pipeline; wall + target replace the test triangle)

- Architecture follows the classic renderer shape — **scene = loader + storage**, no per-object-type C++ classes and no per-object shaders
- New `Mesh` (`src/graphics/mesh.h/cpp`): generic vertex/index buffer pair + index count; geometry comes from `GeometryData` (`geometry.h/cpp`) produced by procedural functions today (`MakeGridWall`, `MakeCube`) — swappable for a file parser (.obj / scene description) later without touching the render path
- New `Material` (`src/graphics/material.h`): pure data (base color, lit/unlit, grid overlay) — one pipeline serves the whole scene, all variation flows through constants
- `SceneObject` (`scene_manager.h`): POD `{ mesh index, material, transform }` — no inheritance, no virtual functions
- `SceneManager`: single VS/PS pair (`object.vs/ps.hlsl`), single pipeline, single binding set (b0 camera + b1 object); per draw: write `ObjectConstants` then `drawIndexed` — transform + material in one buffer, the pattern Phase 3 needs for per-target transforms
- `ObjectConstants` (b1) = ModelMatrix + BaseColor + LightingMode + GridEnabled; binding visibility extended to Vertex|Pixel (PS now reads material params)
- Wall: 20×12 quad at z=+5, unlit + calibration grid using **fixed world-space line width** (first attempt used `fwidth` AA lines on a near-black background — rendered effectively invisible on MoltenVK)
- Target: unit cube, Lambert + ambient; back-face culling deliberate (wrong winding = missing face, tier-1 visible)
- Deleted: SceneObject virtual base class, BackgroundWall/Target classes, per-object wall/target shader pairs, triangle shaders; shader loading lives in shared `shader_utils.h/cpp`
- Wall visibility fix (round 2): **front-face winding on Vulkan** — `Camera::SetFlipY` negates the projection's Y, which mirrors the whole scene and reverses every triangle's winding; with `cullBack` that culled *front* faces (wall fully invisible, cube showed its inner faces). Fixed with `setFrontCounterClockwise(true)` on Vulkan, matching the mirrored projection. Round 1 had also replaced `fwidth`-based AA grid lines (zero-width on MoltenVK) with fixed world-space line width
- Acceptance check: tier 3 — `--frameNum=300` Vulkan run clean (validation silent, NDC sanity log unchanged); tier 1 — fly the god camera: grid wall behind lit cube, depth ordering and culling correct from every angle
