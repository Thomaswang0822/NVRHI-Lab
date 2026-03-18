# Choices Made

## Core Integration
- **NVRHI**: Git submodule at `thirdparty/NVRHI`
- **Library Type**: Static libraries (not dynamic/shared)
  - Reasons: Simpler deployment, single executable, easier debugging, version consistency
- **Build System**: CMake
- **C++ Standard**: C++20 (NVRHI requires 17 or above)

## NVRHI Backends
All 3: DX11, DX12, Vulkan

Though DX11 is legacy, it's used in user's daily work together with DX12.
All backends enabled from Phase 1 onwards (triangle stage), allowing cross-backend comparison from the start.

## NVRHI Optional Features
- **NVAPI**: Set `NVRHI_WITH_NVAPI=ON` (requires NVAPI SDK in `nvapi/` folder)
- **RTXMU**: Set `NVRHI_WITH_RTXMU=ON` (automatic BLAS compaction)

We will decide optional features later.

## Frontend/UI
- **Framework**: wxWidgets
  - Chosen for: Pure C++ (no external build tools like moc), clean and navigable codebase, smaller footprint than Qt
  - Constraint met: Not ImGui (too old-school look)
  - Rationale: User requirement for "code-like" framework - ability to navigate source to understand features and debug issues

## Language Preference
- When explaining concepts, include Mandarin translation in parentheses
- Example: Texture (纹理), Buffer (缓冲区)

## Application Design
- **Concept**: AimLab-like graphics testbed (3D target-shooting game)
- **Rationale**: Simple high-level complexity enables focus on low-level NVRHI API features
- **Key design**: Hardcoded geometries, menu-driven test scenarios, incremental complexity
- **Backend approach**: All backends enabled from Phase 1 (app relaunch to switch, not runtime)
- **Validation**: NVRHI validation layer enabled by default for debugging
- **Geometries**: Hardcoded (no fancy arena switching, focus on rendering API features)

## Device Manager Design
- **Location**: `src/graphics/device_manager`
- **Purpose**: Centralized device and swapchain management across D3D11/D3D12/Vulkan
- **Backend detection**: `GetBackendName()` returns string representation ("D3D11", "D3D12", "Vulkan")
- **Fallback strategy**: Try D3D12 first, fall back to D3D11 if unavailable
- **Swapchain**: Platform-specific creation (DXGI), then wrapped as NVRHI textures
- **Frame synchronization**: Fence-based for D3D12 (per-backbuffer tracking)

## Shader Compilation
- **Compiler**: DXC (DirectX Shader Compiler) - NOT fxc (legacy)
- **Shader Model**: 6.4 (latest supported by DXC)
- **Approach**: CMake-based compilation with custom commands
- **Files**: 
  - `src/shaders/triangle.vs.hlsl` - Vertex shader
  - `src/shaders/triangle.ps.hlsl` - Pixel shader
- **Output**: `bin/shaders/*.cso`
- **CMake module**: `cmake/shaders.cmake`
- **Current Issue (UNRESOLVED)**: When .hlsl files are added to VS via target_sources(), VS tries to compile them with default settings (conflicting with our custom PRE_LINK command). Need to find a way to show shaders in VS Solution Explorer without VS trying to compile them.
