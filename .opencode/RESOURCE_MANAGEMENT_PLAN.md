# Resource Management Refactoring Plan

## Executive Summary

We are using NVRHI to abstract graphics APIs (D3D11/D3D12/Vulkan), but our `DeviceManager` mixes raw backend calls with NVRHI calls throughout. This violates the principle of separation of concerns and creates confusing ownership semantics that lead to memory leaks and crashes.

This document outlines the problems, evaluates options, and proposes a concrete refactoring plan.

---

## 1. Why Current Resource Management is Problematic

### 1.1 Mixing Abstraction Layers

Our current `DeviceManager` code looks like this:

```cpp
bool DeviceManager::CreateD3D12Device() {
    // Raw D3D12 API
    ID3D12Device* device = nullptr;
    D3D12CreateDevice(..., &device);

    ID3D12CommandQueue* commandQueue = nullptr;
    device->CreateCommandQueue(..., &commandQueue);

    // NVRHI API (using raw D3D12 objects)
    nvrhi::d3d12::DeviceDesc nvrhiDesc;
    nvrhiDesc.pDevice = device;
    nvrhiDesc.pGraphicsCommandQueue = commandQueue;

    m_Device = nvrhi::d3d12::createDevice(nvrhiDesc);

    // Later, back to raw D3D12
    m_D3D12Device = device;
    m_D3D12CommandQueue = commandQueue;
}
```

**Problem**: The class constantly switches between raw D3D12 and NVRHI APIs, with no clear boundaries.

### 1.2 Confusing Ownership Semantics

Who owns what?

| Object | Created By | Passed To NVRHI | Released By |
|--------|-----------|-----------------|-------------|
| `ID3D12Device*` | DeviceManager | Yes | DeviceManager |
| `ID3D12CommandQueue*` | DeviceManager | Yes | DeviceManager |
| `IDXGISwapChain*` | DeviceManager | No | DeviceManager |
| NVRHI `DeviceHandle` | NVRHI | N/A | DeviceManager |
| NVRHI `TextureHandle` (backbuffers) | NVRHI (wrapping) | N/A | NVRHI |

**Problem**: When to Release vs when not to Release is unclear. The code has bugs:

```cpp
// In Shutdown():
m_SwapChainBuffers.clear();  // NVRHI releases wrapped textures
m_Device = nullptr;          // NVRHI device destroyed

// But what about the raw D3D12 objects?
DestroyD3D12();              // Releases raw objects
```

This order might be correct, but it's not obvious why.

### 1.3 Memory Leaks and Crashes

The user reported:
1. First run after rebuild: Works
2. Shutdown: Memory leaks detected
3. Second run: Crash in wxWidgets event handler

This pattern suggests heap corruption from improper resource cleanup. The mix of raw COM objects and NVRHI handles makes it hard to verify correct destruction order.

### 1.4 Unclear "Layer 1" Necessity

Looking at NVRHI's `createDevice`:
```cpp
DeviceHandle createDevice(const DeviceDesc& desc) {
    Device* device = new Device(desc);
    return DeviceHandle::Create(device);
}
```

After passing raw pointers to NVRHI, **we never need to touch raw D3D12 again**. Yet our code does:
- Raw fence manipulation
- Raw static_casts throughout
- Raw Release() calls with unclear ownership

---

## 2. Why Separation of Concerns is Critical

### 2.1 Single Responsibility Principle

Each component should have one reason to change.

| Component | Responsibility |
|-----------|---------------|
| Platform Layer | Create raw resources, manage swap chain, present |
| DeviceManager | Use NVRHI for rendering, manage NVRHI resources |
| BasicRenderer | Record rendering commands |

### 2.2 Ownership Clarity

With separation, ownership is explicit:

```
Platform Layer
    Creates: Raw D3D12/D3D11/Vulkan objects
    Owns: Until passed to NVRHI
    Releases: If creation fails, or after NVRHI takes ownership

NVRHI
    Takes: Raw pointers (AddRefs internally)
    Owns: All NVRHI handles
    Releases: On destruction (RefCountPtr handles this)

DeviceManager
    Uses: NVRHI handles only
    Owns: NVRHI device (via RefCountPtr)
    Releases: NVRHI handles in reverse order of creation
```

### 2.3 Easier Debugging

When something goes wrong, we know exactly which layer to examine.

---

## 3. Available Options

### Option A: Backend-Specific DeviceManager Classes

```cpp
class D3D12DeviceManager { /* raw D3D12 + NVRHI mixed */ };
class D3D11DeviceManager { /* raw D3D11 + NVRHI mixed */ };
class VulkanDeviceManager { /* raw Vulkan + NVRHI mixed */ };
```

**Pros**: Simple, backend code isolated
**Cons**: Duplication of NVRHI usage logic across classes

### Option B: Platform Context Abstraction (Chosen)

```cpp
class IPlatformContext {
    virtual nvrhi::DeviceHandle createNvrhiDevice() = 0;
    virtual void createSwapChain(void* window, int w, int h) = 0;
    virtual nvrhi::ITexture* getBackBuffer() = 0;
    virtual void present() = 0;
    virtual void waitForIdle() = 0;
};

class D3D12Context : public IPlatformContext { /* raw D3D12 only */ };
class D3D11Context : public IPlatformContext { /* raw D3D11 only */ };
class VulkanContext : public IPlatformContext { /* raw Vulkan only */ };

class DeviceManager {
    std::unique_ptr<IPlatformContext> m_Platform;
    nvrhi::DeviceHandle m_Device;  // Pure NVRHI!
    // ...
};
```

**Pros**:
- DeviceManager never touches raw APIs
- Clear ownership: Platform → NVRHI → DeviceManager
- Easy to add new backends
- Single DeviceManager class, different Platform implementations

**Cons**:
- Extra abstraction layer (one-time cost)

### Option C: Simplified - Isolate Raw, Then Pure NVRHI

Keep current structure but add comments and organize raw calls into separate functions.

**Pros**: Minimal refactoring
**Cons**: Raw pointers still scattered, doesn't fully solve the problem

---

## 4. Why We Choose Option B

### 4.1 Matches NVRHI's Design Philosophy

NVRHI abstracts rendering APIs. Our DeviceManager should abstract platform-specific device creation. Option B extends this philosophy to its logical conclusion.

### 4.2 Future-Proof

When we add Vulkan support, we only create a new `VulkanContext`. The rest of the codebase (DeviceManager, BasicRenderer, etc.) requires **zero changes**.

### 4.3 Easier Testing

We can test NVRHI usage logic with a mock platform context, without requiring actual GPU.

### 4.4 Cleaner Ownership Model

```
┌─────────────────────────────────────────────────────────────┐
│  DeviceManager (Pure NVRHI)                                 │
│  - Creates NVRHI device from Platform's raw handles         │
│  - Manages NVRHI resources (textures, buffers, pipelines)    │
│  - Never touches raw D3D12/D3D11/Vulkan                    │
└─────────────────────────────────────────────────────────────┘
                            ▲
                            │ Provides NVRHI device
                            │
┌─────────────────────────────────────────────────────────────┐
│  IPlatformContext (Interface)                               │
│  - createNvrhiDevice(): Returns NVRHI device handle         │
│  - createSwapChain(): Sets up presentation                  │
│  - getBackBuffer(): Returns NVRHI texture handle            │
│  - present(): Swap buffers                                  │
│  - waitForIdle(): GPU synchronization                       │
└─────────────────────────────────────────────────────────────┘
                            │
        ┌───────────────────┼───────────────────┐
        ▼                   ▼                   ▼
┌───────────────┐  ┌───────────────┐  ┌───────────────┐
│ D3D12Context  │  │ D3D11Context  │  │ VulkanContext │
│ (Raw D3D12)   │  │ (Raw D3D11)   │  │ (Raw Vulkan)  │
└───────────────┘  └───────────────┘  └───────────────┘
```

---

## 5. Code Refactoring Plan

### 5.1 New File Structure

```
src/graphics/
├── platform/
│   ├── platform_context.h       # Interface definition
│   ├── d3d12_context.h/cpp      # D3D12 implementation
│   ├── d3d11_context.h/cpp      # D3D11 implementation (stub)
│   └── vulkan_context.h/cpp     # Vulkan implementation (stub)
├── device_manager.h/cpp          # Refactored - pure NVRHI
├── basic_renderer.h/cpp          # Unchanged
└── ...
```

### 5.2 IPlatformContext Interface

```cpp
// platform_context.h
#pragma once

#include <nvrhi/nvrhi.h>

namespace nvrhi_lab {

class IPlatformContext {
public:
    virtual ~IPlatformContext() = default;

    // Creates NVRHI device and returns it
    virtual nvrhi::DeviceHandle createNvrhiDevice() = 0;

    // Swap chain management
    virtual void createSwapChain(void* windowHandle, int width, int height, int backBufferCount) = 0;
    virtual nvrhi::ITexture* getBackBuffer(size_t index) = 0;
    virtual size_t getCurrentBackBufferIndex() = 0;
    virtual void present() = 0;

    // GPU synchronization
    virtual void waitForIdle() = 0;

    // Info
    virtual int getWidth() const = 0;
    virtual int getHeight() const = 0;
    virtual const char* getBackendName() const = 0;
};

} // namespace nvrhi_lab
```

### 5.3 D3D12Context Implementation

```cpp
// d3d12_context.h/cpp
class D3D12Context : public IPlatformContext {
public:
    D3D12Context(const PlatformDesc& desc);
    ~D3D12Context();

    nvrhi::DeviceHandle createNvrhiDevice() override;
    void createSwapChain(...) override;
    // ... other methods

private:
    // Raw D3D12 objects - owned, released in destructor
    Microsoft::WRL::ComPtr<IDXGIFactory2> m_Factory;
    Microsoft::WRL::ComPtr<ID3D12Device> m_Device;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CommandQueue;
    Microsoft::WRL::ComPtr<IDXGISwapChain3> m_SwapChain;
    Microsoft::WRL::ComPtr<ID3D12Fence> m_Fence;
    HANDLE m_FenceEvent = nullptr;
    uint64_t m_FenceValue = 0;

    nvrhi::DeviceHandle m_NvrhiDevice;  // From createNvrhiDevice()
    std::vector<nvrhi::TextureHandle> m_SwapChainBuffers;
};
```

### 5.4 DeviceManager Refactored

```cpp
// device_manager.h/cpp
class DeviceManager {
public:
    DeviceManager(GraphicsBackend backend);
    ~DeviceManager();

    bool Initialize(void* windowHandle, int width, int height);
    void Shutdown();

    // NVRHI device access
    nvrhi::IDevice* GetDevice() const { return m_Device; }

    // Frame management
    void BeginFrame();
    void Present();

    // Backbuffer access
    nvrhi::ITexture* GetCurrentBackBuffer();
    int GetWidth() const { return m_Platform->getWidth(); }
    int GetHeight() const { return m_Platform->getHeight(); }

private:
    GraphicsBackend m_Backend;
    std::unique_ptr<IPlatformContext> m_Platform;
    nvrhi::DeviceHandle m_Device;  // Pure NVRHI
    nvrhi::ValidationLayer::IValidationLayer* m_ValidationLayer = nullptr;
};
```

### 5.5 Destruction Order (Guaranteed Safe)

```cpp
DeviceManager::~DeviceManager() {
    Shutdown();
}

void DeviceManager::Shutdown() {
    if (!m_Initialized) return;

    // 1. Clear NVRHI resources (BasicRenderer will be destroyed before this)
    m_Device = nullptr;  // NVRHI releases everything it owns

    // 2. Platform context destroyed, releases raw D3D12/D3D11/Vulkan
    m_Platform.reset();

    m_Initialized = false;
}
```

### 5.6 Key Changes Summary

| Before | After |
|--------|-------|
| Mixed raw + NVRHI in one class | Platform handles raw, DeviceManager pure NVRHI |
| Unclear Release() ownership | Raw Release() in Platform destructor |
| Backend selection via switch() | Polymorphic via IPlatformContext |
| Hard to add Vulkan | Easy - just implement VulkanContext |

---

## 6. Implementation Order

1. **Create `platform_context.h`** - Define interface
2. **Create `d3d12_context.h/cpp`** - Implement D3D12 backend
3. **Refactor `device_manager.h/cpp`** - Use IPlatformContext
4. **Update `basic_renderer.h/cpp`** - No changes needed
5. **Update `my_app.h/cpp`** - No changes needed
6. **Create `d3d11_context.h/cpp`** - Stub for future
7. **Create `vulkan_context.h/cpp`** - Stub for future
8. **Test** - Verify memory leaks fixed

---

## 7. Success Criteria

- [ ] DeviceManager contains **zero** raw D3D12/D3D11/Vulkan pointers
- [ ] All raw API calls isolated to platform context classes
- [ ] No memory leaks on shutdown
- [ ] Multiple runs don't crash
- [ ] Can switch backends by changing one enum value
- [ ] Adding new backend requires creating new class, not modifying existing

See **RULES.md** for Resource Management principles (RAII, ownership rules).
