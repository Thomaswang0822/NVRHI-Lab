#pragma once

#include <cstdint>

namespace nvrhi_lab {

// macOS 平台辅助（Objective-C++ 实现，见 macos_surface.mm）

// 为给定 NSView 挂接一个 CAMetalLayer 并返回该 layer 的原始指针，
// 供 vkCreateMetalSurfaceEXT（VK_EXT_metal_surface）使用。
// layer 由 view 持有，调用方无需管理生命周期。
void* CreateMetalLayerForView(void* nsView);

// 查询 NSView 客户区的像素尺寸（已乘以 backingScaleFactor，用于 Retina）。
void GetViewClientPixelSize(void* nsView, uint32_t& width, uint32_t& height);

} // namespace nvrhi_lab
