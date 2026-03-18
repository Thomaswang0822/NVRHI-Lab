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
