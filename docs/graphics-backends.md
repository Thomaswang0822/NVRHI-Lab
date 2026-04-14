# Graphics Backends Comparison

This document provides an engineering-focused comparison of graphics APIs used in this project: D3D11, D3D12, and Vulkan. Understanding these differences helps explain why NVRHI's abstraction is valuable.

## Philosophy

| Aspect | D3D11 | D3D12 | Vulkan |
|--------|-------|-------|--------|
| **Abstraction** | High | Low | Lowest |
| **Driver work** | Heavy | Moderate | Minimal |
| **Control** | Limited | Fine-grained | Explicit |
| **Error handling** | Runtime checks | Debug layer | Validation layers |

**Key insight:** Vulkan is "explicit by design" - the driver does almost nothing automatically. You must specify everything.

---

## 1. Shader Format

| API | Shader Format | Compiler | Extension |
|-----|---------------|----------|-----------|
| D3D11 | DXBC (bytecode) | FXC | `.cso` |
| D3D12 | DXIL (bytecode) | DXC | `.cso` |
| **Vulkan** | **SPIR-V** | DXC (with `-spirv`) or glslang | `.spv` |

### Compilation Differences

```
D3D11:  HLSL → FXC → DXBC (.cso)
D3D12:  HLSL → DXC → DXIL (.cso)
Vulkan: HLSL → DXC -spirv → SPIR-V (.spv)
```

**Good news:** You can use the same HLSL source! DXC supports cross-compiling HLSL to SPIR-V.

**Gotcha:** Some HLSL features need adjustments for Vulkan:
- Semantic names (`SV_Position` → `gl_Position` in GLSL, but DXC handles this)
- Resource binding models differ significantly

---

## 2. Pipeline State Object (PSO)

### D3D12 Approach
```cpp
// D3D12: Monolithic PSO created upfront
D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
psoDesc.pRootSignature = rootSignature;
psoDesc.VS = { vsBytecode, vsSize };
psoDesc.PS = { psBytecode, psSize };
psoDesc.BlendState = blendDesc;
psoDesc.RasterizerState = rasterizerDesc;
psoDesc.DepthStencilState = depthStencilDesc;
psoDesc.InputLayout = { inputLayout, inputLayoutCount };
// ... many more states
device->CreateGraphicsPipelineState(&psoDesc, &pso);
```

### Vulkan Approach
```cpp
// Vulkan: Even more explicit, with pipeline layout separate
VkGraphicsPipelineCreateInfo pipelineInfo = {};
pipelineInfo.stageCount = 2;
pipelineInfo.pStages = shaderStages;  // VkPipelineShaderStageCreateInfo[]
pipelineInfo.pVertexInputState = &vertexInputInfo;
pipelineInfo.pInputAssemblyState = &inputAssembly;
pipelineInfo.pViewportState = &viewportInfo;
pipelineInfo.pRasterizationState = &rasterizer;
pipelineInfo.pMultisampleState = &multisampling;
pipelineInfo.pDepthStencilState = &depthStencil;
pipelineInfo.pColorBlendState = &colorBlending;
pipelineInfo.layout = pipelineLayout;  // Separate object!
pipelineInfo.renderPass = renderPass;  // Must match render pass!
vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineInfo, nullptr, &pipeline);
```

**Key differences:**

| Aspect | D3D12 | Vulkan |
|--------|-------|--------|
| Render pass | Implicit | Explicit `VkRenderPass` |
| Pipeline layout | Root signature | `VkPipelineLayout` (separate object) |
| Subpass support | Limited | First-class concept |

---

## 3. Resource Binding Model

This is where the biggest differences lie.

### D3D12: Root Signature + Descriptor Heaps
```
Root Signature (up to 64 DWORDs)
├── Root Constants (inline values)
├── Root Descriptors (inline CBV/SRV/UAV pointers)
└── Descriptor Tables (pointers into heaps)

Descriptor Heap
├── CBV SRV UAV Heap (shader-visible)
└── Sampler Heap (separate)
```

### Vulkan: Descriptor Sets + Pipeline Layout
```
Pipeline Layout
├── Set 0: Per-frame resources
├── Set 1: Per-material resources
├── Set 2: Per-object resources
└── Push Constants (small inline data, like root constants)

Descriptor Pool → Allocates Descriptor Sets
├── Descriptor Set 0
├── Descriptor Set 1
└── Descriptor Set 2
```

**Key differences:**

| Aspect | D3D12 | Vulkan |
|--------|-------|--------|
| Binding frequency | Root signature levels | Descriptor set indices |
| Descriptor updates | Copy to heap | Write to set or update after bind |
| Sampler binding | Separate heap | Part of descriptor set |
| Max bindings | Limited by heap size | Limited by device limits |

### Binding Comparison Example

```cpp
// D3D12: Bind descriptor table
commandList->SetGraphicsRootDescriptorTable(0, gpuHandle);

// Vulkan: Bind descriptor set
vkCmdBindDescriptorSets(
    commandBuffer,
    VK_PIPELINE_BIND_POINT_GRAPHICS,
    pipelineLayout,
    0,              // firstSet
    1,              // descriptorSetCount
    &descriptorSet,
    0, nullptr      // dynamic offsets
);
```

---

## 4. Memory Management

### D3D12: Heaps + Resources
```cpp
// D3D12: Resource implicitly in a heap, or placed explicitly
D3D12_HEAP_PROPERTIES heapProps = {};
heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;  // GPU-only

D3D12_RESOURCE_DESC resourceDesc = {};
device->CreateCommittedResource(
    &heapProps,
    D3D12_HEAP_FLAG_NONE,
    &resourceDesc,
    D3D12_RESOURCE_STATE_COMMON,
    nullptr,
    &resource
);
```

### Vulkan: Explicit Memory Allocation
```cpp
// Vulkan: Two-step process
VkBuffer buffer;
vkCreateBuffer(device, &bufferInfo, nullptr, &buffer);

VkMemoryRequirements memReqs;
vkGetBufferMemoryRequirements(device, buffer, &memReqs);

VkMemoryAllocateInfo allocInfo = {};
allocInfo.allocationSize = memReqs.size;
allocInfo.memoryTypeIndex = FindMemoryType(memReqs.memoryTypeBits, 
                                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

VkDeviceMemory memory;
vkAllocateMemory(device, &allocInfo, nullptr, &memory);

vkBindBufferMemory(device, buffer, memory, 0);
```

**Key differences:**

| Aspect | D3D12 | Vulkan |
|--------|-------|--------|
| Allocation | Often implicit | Always explicit |
| Memory types | 3 heap types | Many memory types (device-specific) |
| Sub-allocation | Recommended | Essential for performance |
| Memory mapping | `Map()` on resource | `vkMapMemory()` on device memory |

---

## 5. Synchronization

### D3D12: Fences
```cpp
// D3D12: Signal and wait on fence
commandQueue->Signal(fence, fenceValue);
fence->SetEventOnCompletion(fenceValue, event);
WaitForSingleObject(event, INFINITE);
```

### Vulkan: Fences + Semaphores + Barriers
```cpp
// Vulkan: Multiple synchronization primitives

// 1. Fences (CPU-GPU sync)
vkQueueSubmit(queue, 1, &submitInfo, fence);
vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX);

// 2. Semaphores (GPU-GPU sync between queues)
VkSubmitInfo submitInfo = {};
submitInfo.waitSemaphoreCount = 1;
submitInfo.pWaitSemaphores = &imageAvailableSemaphore;
submitInfo.signalSemaphoreCount = 1;
submitInfo.pSignalSemaphores = &renderFinishedSemaphore;

// 3. Pipeline Barriers (within command buffer)
vkCmdPipelineBarrier(
    commandBuffer,
    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
    0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier
);
```

**Key differences:**

| Aspect | D3D12 | Vulkan |
|--------|-------|--------|
| CPU-GPU sync | Fence | Fence |
| GPU-GPU sync | Implicit (single queue) | Semaphore (explicit) |
| Resource transitions | `ResourceBarrier()` | `vkCmdPipelineBarrier()` |
| Execution ordering | Mostly implicit | Entirely explicit |

---

## 6. Command Buffers

### D3D12: Command Lists
```cpp
ID3D12GraphicsCommandList* cmdList;
device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, 
                          allocator, nullptr, &cmdList);
cmdList->Close();
queue->ExecuteCommandLists(1, &cmdList);
```

### Vulkan: Command Buffers
```cpp
VkCommandBuffer cmdBuffer;
VkCommandBufferAllocateInfo allocInfo = {};
allocInfo.commandPool = commandPool;
allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
vkAllocateCommandBuffers(device, &allocInfo, &cmdBuffer);

VkCommandBufferBeginInfo beginInfo = {};
vkBeginCommandBuffer(cmdBuffer, &beginInfo);
// ... record commands ...
vkEndCommandBuffer(cmdBuffer);

VkSubmitInfo submitInfo = {};
submitInfo.commandBufferCount = 1;
submitInfo.pCommandBuffers = &cmdBuffer;
vkQueueSubmit(queue, 1, &submitInfo, fence);
```

**Key differences:**

| Aspect | D3D12 | Vulkan |
|--------|-------|--------|
| Pool management | Per-allocation | `VkCommandPool` (explicit) |
| Reset | `Reset()` allocator | `vkResetCommandPool()` |
| Reuse | Reset + re-record | Same, or use secondary buffers |

---

## 7. Swap Chain & Presentation

### D3D12
```cpp
IDXGISwapChain3* swapChain;
swapChain->Present(vsync ? 1 : 0, 0);
UINT backBufferIndex = swapChain->GetCurrentBackBufferIndex();
```

### Vulkan
```cpp
VkSwapchainKHR swapchain;
VkPresentInfoKHR presentInfo = {};
presentInfo.swapchainCount = 1;
presentInfo.pSwapchains = &swapchain;
presentInfo.pImageIndices = &imageIndex;
vkQueuePresentKHR(queue, &presentInfo);

// Acquire next image explicitly
vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, 
                       semaphore, fence, &imageIndex);
```

**Key differences:**

| Aspect | D3D12 | Vulkan |
|--------|-------|--------|
| Image acquisition | Implicit in Present | Explicit `vkAcquireNextImageKHR` |
| Synchronization | Automatic | Must use semaphore/fence |
| Surface format | DXGI_FORMAT | VkFormat (same values, different enum) |

---

## 8. Instance & Device Creation

### D3D12
```cpp
// D3D12: Simple factory pattern
IDXGIFactory6* factory;
CreateDXGIFactory2(flags, &factory);

ID3D12Device* device;
D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, &device);
```

### Vulkan
```cpp
// Vulkan: Explicit instance + physical device selection + logical device
VkInstance instance;
VkInstanceCreateInfo createInfo = {};
createInfo.enabledLayerCount = layerCount;
createInfo.ppEnabledLayerNames = layers;
createInfo.enabledExtensionCount = extCount;
createInfo.ppEnabledExtensionNames = extensions;
vkCreateInstance(&createInfo, nullptr, &instance);

// Enumerate and select physical device
VkPhysicalDevice physicalDevice;
uint32_t deviceCount;
vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
std::vector<VkPhysicalDevice> devices(deviceCount);
vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
// ... select based on properties, queue families, extensions ...

// Create logical device with queues
VkDevice device;
VkDeviceQueueCreateInfo queueCreateInfo = {};
queueCreateInfo.queueFamilyIndex = graphicsQueueFamilyIndex;
queueCreateInfo.queueCount = 1;
float queuePriority = 1.0f;
queueCreateInfo.pQueuePriorities = &queuePriority;

VkDeviceCreateInfo deviceCreateInfo = {};
deviceCreateInfo.queueCreateInfoCount = 1;
deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device);
```

---

## 9. API Binding Mechanisms

One fundamental difference between DirectX and Vulkan is how API functions are accessed at runtime.

### DirectX: COM (Component Object Model)

DirectX uses Microsoft's COM, a binary standard for object-oriented interfaces. When you call `D3D12CreateDevice()`, you receive a pointer to a COM interface:

```
Application
    │
    ▼
┌─────────────────────────────────────┐
│     ID3D12Device (COM Interface)    │
│  ┌───────────────────────────────┐  │
│  │ Virtual Function Table (vtable)│  │
│  │ ├── CreateBuffer()            │  │
│  │ ├── CreateTexture()           │  │
│  │ ├── CreateCommandList()       │  │
│  │ └── ...                       │  │
│  └───────────────────────────────┘  │
└─────────────────────────────────────┘
    │
    ▼
Driver Implementation
```

**Key characteristics:**
- **Implicit binding**: Function pointers come pre-loaded in the vtable
- **Reference counting**: `AddRef()` / `Release()` for lifetime management
- **Interface inheritance**: `ID3D12Device` extends `ID3D12Object` extends `IUnknown`
- **No manual loading**: The driver provides everything through the interface

You've been using COM without realizing it! Every `device->CreateBuffer()` call goes through the vtable automatically.

### Vulkan: Explicit Function Loading

Vulkan has no equivalent to COM. Instead, every function must be loaded explicitly via `vkGetInstanceProcAddr`:

```
Application
    │
    ▼
┌─────────────────────────────────────┐
│         vulkan-1.dll                │
│  Contains: vkGetInstanceProcAddr    │
└─────────────────────────────────────┘
    │
    ▼ vkGetInstanceProcAddr("vkCreateDevice")
┌─────────────────────────────────────┐
│      Function Pointer Storage       │
│  PFN_vkCreateDevice vkCreateDevice  │
│  PFN_vkCreateBuffer vkCreateBuffer  │
│  ... hundreds more ...              │
└─────────────────────────────────────┘
    │
    ▼
Driver Implementation
```

**Two API styles:**

| Style | Header | Approach |
|-------|--------|----------|
| **C API** | `<vulkan/vulkan.h>` | Manage function pointers yourself, or use a loader |
| **C++ API (Vulkan-Hpp)** | `<vulkan/vulkan.hpp>` | Automatic dispatcher manages pointers |

### Vulkan-Hpp Dynamic Dispatcher

Vulkan-Hpp provides a global `defaultDispatchLoaderDynamic` object that stores all function pointers. This is what NVRHI uses internally:

```
┌─────────────────────────────────────────────────────────────────┐
│                        vulkan-1.dll                             │
│  Contains: vkGetInstanceProcAddr + actual driver functions      │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│                    vk::DynamicLoader                            │
│  Loads vulkan-1.dll, retrieves vkGetInstanceProcAddr            │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│              VULKAN_HPP_DEFAULT_DISPATCHER                      │
│  (global DispatchLoaderDynamic object)                          │
│                                                                 │
│  Before initialization:                                         │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │ PFN_vkCreateInstance vkCreateInstance = nullptr;        │   │
│  │ PFN_vkCreateDevice vkCreateDevice = nullptr;            │   │
│  │ PFN_vkCreateSemaphore vkCreateSemaphore = nullptr; ❌    │   │
│  │ ... hundreds more ...                                   │   │
│  └─────────────────────────────────────────────────────────┘   │
│                                                                 │
│              After .init(instance, vkGetInstanceProcAddr)       │
│                                                                 │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │ PFN_vkCreateInstance vkCreateInstance = 0x7FF...; ✅     │   │
│  │ PFN_vkCreateDevice vkCreateDevice = 0x7FF...; ✅         │   │
│  │ PFN_vkCreateSemaphore vkCreateSemaphore = 0x7FF...; ✅   │   │
│  │ ... all populated ...                                   │   │
│  └─────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│                    Your Code / NVRHI                            │
│                                                                 │
│  vk::createInstance(...)     → dispatcher.vkCreateInstance()   │
│  device.createSemaphore(...) → dispatcher.vkCreateSemaphore()  │
└─────────────────────────────────────────────────────────────────┘
```

**Critical requirement**: The dispatcher must be initialized after creating a Vulkan instance, or all function pointers remain null.

### Static vs Shared Library Build

NVRHI's behavior differs based on build mode:

| Build Mode | Macro | Who initializes dispatcher? |
|------------|-------|----------------------------|
| **Shared Library** | `NVRHI_BUILD_SHARED=ON` | NVRHI itself (in `nvrhi::vulkan::createDevice`) |
| **Static Library** | (default) | **The application** must call `dispatcher.init()` |

When using NVRHI as a static library (the default), your application must:
1. Define `VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE` (provides storage)
2. Call `VULKAN_HPP_DEFAULT_DISPATCHER.init(instance, vkGetInstanceProcAddr)` after creating the instance

### Comparison Summary

| Aspect | D3D11/12 (COM) | Vulkan C API | Vulkan-Hpp |
|--------|----------------|--------------|------------|
| **Function access** | Vtable (implicit) | Manual via `vkGetInstanceProcAddr` | Automatic via dispatcher |
| **Initialization** | `D3D12CreateDevice()` | `vkCreateInstance()` + load functions | `vkCreateInstance()` + `dispatcher.init()` |
| **Lifetime management** | `AddRef()` / `Release()` | Manual `vkDestroy*()` | RAII or manual |
| **Error handling** | `HRESULT` return | `VkResult` return | Exceptions or `vk::Result` |

---

## 10. Vulkan Features Required by NVRHI

NVRHI requires specific Vulkan features to be enabled at device creation time. These are not optional - NVRHI's internal implementation depends on them.

### Required Features

| Feature | Vulkan Version | Purpose |
|---------|----------------|---------|
| `timelineSemaphore` | 1.2 | Queue synchronization with timeline semaphores |
| `dynamicRendering` | 1.3 | Render-pass-less pipeline creation |
| `synchronization2` | 1.3 | Simplified synchronization API |

### Enabling Features

Features are enabled via the `pNext` chain in `VkDeviceCreateInfo`:

```
VkDeviceCreateInfo
    └── pNext → VkPhysicalDeviceVulkan13Features
                    ├── dynamicRendering = VK_TRUE
                    ├── synchronization2 = VK_TRUE
                    └── pNext → VkPhysicalDeviceVulkan12Features
                                    └── timelineSemaphore = VK_TRUE
```

### API Version Requirements

- **Minimum**: Vulkan 1.3 (for `dynamicRendering`)
- **NVRHI compile-time**: VK_HEADER_VERSION >= 318 (Vulkan SDK 1.4.318+)

### Two-Phase Dispatcher Initialization

Vulkan-Hpp requires the dispatcher to be initialized in two phases:

1. **Phase 1** (before `vkCreateInstance`): Load global functions
   - `vkEnumerateInstanceLayerProperties`
   - `vkEnumerateInstanceExtensionProperties`
   - `vkCreateInstance`

2. **Phase 2** (after `vkCreateInstance`): Load instance functions
   - `vkDestroyInstance`
   - `vkEnumeratePhysicalDevices`
   - `vkCreateDevice`
   - All other instance-level functions

---

## Summary: What This Means for NVRHI-Lab

| Component | D3D11 | D3D12 | Vulkan |
|-----------|-------|-------|--------|
| **Shader compilation** | FXC → DXBC | DXC → DXIL | DXC → SPIR-V |
| **Context creation** | Device + Context | Device + Queue | Instance + PhysicalDevice + Device + Queue |
| **Swap chain** | DXGI swap chain | DXGI swap chain | VkSwapchainKHR |
| **Memory** | Implicit | Heaps | Explicit allocation |
| **Synchronization** | Mostly implicit | Fences | Fences + Semaphores + Barriers |

**NVRHI's role:** NVRHI abstracts most of these differences! Your backend implementation will:
1. Create the native API objects (instance, device, swap chain)
2. Wrap native objects in NVRHI handles
3. Let NVRHI handle the translation layer

The good news: NVRHI already has a Vulkan backend (`nvrhi_vk`), so you mainly need to set up the Vulkan infrastructure, not implement the full API.

---

## Further Reading

- [NVRHI Guide](nvrhi-guide.md) - How to use NVRHI's unified API
- [Architecture](architecture.md) - Project structure and component interactions
