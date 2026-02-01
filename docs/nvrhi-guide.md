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

Creating a NVRHI device wraps an underlying graphics API device (D3D11, D3D12, or Vulkan).

### DX12 Example
```cpp
nvrhi::d3d12::DeviceDesc deviceDesc;
deviceDesc.errorCB = g_MyMessageCallback;
deviceDesc.pDevice = d3d12Device;
deviceDesc.pGraphicsCommandQueue = d3d12GraphicsCommandQueue;

nvrhi::DeviceHandle nvrhiDevice = nvrhi::d3d12::createDevice(deviceDesc);
```

### Validation Layer
```cpp
nvrhi::DeviceHandle nvrhiValidationLayer = nvrhi::validation::createValidationLayer(nvrhiDevice);
nvrhiDevice = nvrhiValidationLayer;
```

**Key Points:**
- NVRHI abstracts backend-specific device creation
- Validation layer wraps device for debugging
- DeviceHandle manages reference counting automatically

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

*Coming soon...*

---

## Framebuffers

*Coming soon...*
