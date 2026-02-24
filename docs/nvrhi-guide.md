# NVRHI Guide

This document provides a quick reference for NVRHI concepts as they're learned throughout the project.

> **Note**: This guide will be populated incrementally as we explore NVRHI features.

## Table of Contents

- [Device Creation](#device-creation)
- [Resources](#resources)
- [Pipeline State Objects](#pipeline-state-objects)
- [Command Lists](#command-lists)
- [Swap Chain](#swap-chain)
- [Framebuffers](#framebuffers)

---

## Device Creation

NVRHI acts as a **unified abstraction layer** over multiple graphics APIs. Instead of writing different code for D3D11, D3D12, or Vulkan, you create a single NVRHI device that works the same way regardless of the underlying API.

### The Concept

Think of NVRHI as a **translator**: you write rendering commands in NVRHI's language, and it translates those commands into D3D11, D3D12, or Vulkan calls behind the scenes.

```cpp
// One interface, multiple backends
nvrhi::IDevice* device = CreateDevice(backendChoice);  // D3D11, D3D12, or Vulkan

// Same code works regardless of backend
device->createTexture(...);
device->createBuffer(...);
```

### Backend-Specific Requirements

While NVRHI unifies the API, each backend has different prerequisites:

| Backend | What NVRHI Needs From You |
|---------|---------------------------|
| **D3D11** | Device context (immediate mode API) |
| **D3D12** | Device + Command Queue (explicit submission API) |
| **Vulkan** | Instance + Physical Device + Logical Device + Queues |

### Validation Layer

NVRHI provides an optional **validation layer** that catches common mistakes:
- Resource state mismatches (e.g., trying to render to a texture in the wrong state)
- Missing transitions between read/write operations
- Invalid API usage

Enable this during development for helpful error messages; disable in release builds for performance.

**Key Concepts:**
- **Abstraction**: Write once, run on any supported backend
- **Reference counting**: NVRHI handles object lifetime automatically
- **Validation**: Optional debugging layer helps catch API misuse
- **No overhead**: Release builds have zero abstraction cost

---

## Resources

*Coming soon...*

---

## Pipeline State Objects

*Coming soon...*

---

## Command Lists

*Coming soon...*

---

## Swap Chain

### The Challenge

The **swap chain** (the buffer that gets displayed on screen) is one of the few things NVRHI doesn't abstract. Why? Because swap chains are deeply tied to the windowing system and vary significantly between platforms and APIs.

### The Solution: Texture Wrapping

Instead of creating swap chains, NVRHI lets you **wrap existing native textures**. You create the swap chain using platform APIs (DXGI on Windows), then give NVRHI "handles" to work with:

```cpp
// You create the swap chain (platform-specific)
SwapChain* swapChain = CreatePlatformSwapChain(...);

// You give NVRHI access to the backbuffer textures
for (each backbuffer) {
    NativeTexture* nativeTexture = swapChain->GetBackBuffer(index);
    nvrhi::TextureHandle nvrhiTexture = device->wrapNativeTexture(nativeTexture);
}
```

### Why This Design?

1. **Flexibility**: You control presentation (vsync, fullscreen, etc.)
2. **Simplicity**: NVRHI focuses on rendering, not window management
3. **Interoperability**: Easy to integrate with existing window frameworks (Qt, wxWidgets, etc.)

**Key Concepts:**
- **Separation of concerns**: Window/display management stays separate from rendering
- **Native interop**: NVRHI can work with existing native resources
- **Resource states**: Swap chain textures need special state handling for presentation

---

## Framebuffers

*Coming soon...*
