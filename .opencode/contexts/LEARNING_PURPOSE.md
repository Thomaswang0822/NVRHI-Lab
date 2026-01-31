# Learning Purpose

## Primary Goal
Comprehensively understand graphics API fundamentals through NVRHI abstraction layer.

## What We Learn
- **NVRHI API concepts**: Resources, pipelines, bindings, states, etc.
- **Backend-agnostic fundamentals**: e.g., "What is patchlist primitive type" (not "how D3D12 vs Vulkan handle it")
- **Graphics API patterns**: Resource state tracking, descriptor management, command recording

## What We Don't Learn (in this project)
- Backend-specific implementation details (D3D12/Vulkan internals)
- Advanced rendering algorithms (deferred shading, path tracing, etc.)
- Production-grade engine architecture

## Learning Boundary
- **Stop at**: NVRHI API calls
- **Explore deeper**: Use "go to definition" to peek at raw D3D/Vulkan code
- **Read documentation**: Native API docs for important concepts

## Knowledge Background
- Language: Native Mandarin, learned CS/coding in English
- Programming: C++, CMake, CUDA, HLSL (very familiar)
- Expertise: Graphics + Software Engineering
  - Familiar: Software & algorithm perspective, computer architecture
  - Weak: Basic hardware knowledge
