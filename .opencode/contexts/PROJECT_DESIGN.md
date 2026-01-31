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
