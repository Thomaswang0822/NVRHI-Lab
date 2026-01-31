# Choices Made

## Core Integration
- **NVRHI**: Git submodule at `thirdparty/NVRHI`
- **Library Type**: Static libraries (not dynamic/shared)
  - Reasons: Simpler deployment, single executable, easier debugging, version consistency
- **Build System**: CMake
- **C++ Standard**: C++20 (NVRHI requires 17 or above)

## NVRHI Backends
All 3: DX12, DX12, Vulkan

Though DX11 is legacy, it's used in user's daily work together with DX12.

## NVRHI Optional Features
- **NVAPI**: Set `NVRHI_WITH_NVAPI=ON` (requires NVAPI SDK in `nvapi/` folder)
- **RTXMU**: Set `NVRHI_WITH_RTXMU=ON` (automatic BLAS compaction)

We will decide optional features later.

## Frontend/UI
- **Decision pending**: Modern GUI framework (Qt, Brisk, or other)
- **Constraint**: NOT ImGui (too old-school look)

## Language Preference
- When explaining concepts, include Mandarin translation in parentheses
- Example: Texture (纹理), Buffer (缓冲区)
