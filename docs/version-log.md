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
- **D3D11/D3D12 working with camera transformation; Vulkan has descriptor set validation error**
