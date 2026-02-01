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
