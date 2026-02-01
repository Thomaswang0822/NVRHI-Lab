# Project Design

## Overview
A graphics learning laboratory using NVRHI (NV Rendering Hardware Interface) as the core abstraction layer to deeply understand modern graphics API concepts.

## Core Philosophy
- **"Be simpler but go deeper"**: Focus on fundamental concepts rather than advanced rendering algorithms
- **Comprehensive coverage**: If NVRHI supports 8 primitive types, implement all 8
- **Backend-agnostic learning**: Stop at NVRHI API level, not dive into backend implementation details

## Primary Focus
Learn graphics API features (D3D11, D3D12, Vulkan) that NVRHI abstracts:
- Resource management (textures, buffers, etc.)
- Pipeline states and shader compilation
- Resource binding systems
- Command lists and execution
- Render targets and barriers
- Ray tracing (BLAS/TLAS)
- Compute shaders

## Example Learning Path
Previously used textures without deep understanding. Goal: Understand concepts like mipmaps from NVRHI API perspective, then explore native APIs when needed for clarification.

## Application Concept: AimLab-like Testbed

**Overview**: The application is designed as a 3D target-shooting testbed similar to AimLab, serving as an interactive laboratory for graphics API experimentation.

**Why AimLab Design:**
- **Simple high-level complexity**: Basic scene loading and movement, leaving focus on low-level rendering details
- **Natural experimentation surface**: Change textures on targets/background to test different NVRHI resource types
- **Clear success criteria**: Interactive 3D graphics with clickable targets demonstrates working graphics + input handling
- **Incremental learning path**: Start with static scene, add movement, add effects gradually

**Learning Goals through Testbed:**
- **Phase 1**: NVRHI device creation, swap chain, pipeline state objects
- **Phase 2**: Buffers, textures, resource bindings, descriptor sets
- **Phase 3**: Dynamic resource updates, buffer mapping, hit detection
- **Phase 4**: Pipeline state variations, resource state transitions
- **Phase 5**: Ray tracing (BLAS/TLAS), compute shaders, advanced execution

**Key Design Decisions:**
- **Hardcoded geometries**: No fancy arena switching, focus on rendering API features
- **All backends enabled**: DX11, DX12, Vulkan available from triangle stage onwards (app relaunch to switch)
- **Menu-driven exploration**: wxWidgets menu to switch between test scenarios (texture types, primitive types, etc.)
- **Validation by default**: NVRHI validation layer enabled for debugging

**Final Goal**: Playable AimLab-like experience with minimal fancy features, ultimately allowing users to choose different NVRHI backends and options from menu bar.
