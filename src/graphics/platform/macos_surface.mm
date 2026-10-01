#include "macos_surface.h"

#import <AppKit/AppKit.h>
#import <QuartzCore/QuartzCore.h>

namespace nvrhi_lab {

void* CreateMetalLayerForView(void* nsView) {
    if (!nsView) {
        return nullptr;
    }

    NSView* view = (__bridge NSView*)nsView;

    // 先令 view 变为 layer-backed，再把自动创建的 layer 替换为 CAMetalLayer，
    // MoltenVK 会通过 VK_EXT_metal_surface 直接使用该 layer。
    view.wantsLayer = YES;
    CAMetalLayer* layer = [CAMetalLayer layer];

    // Retina：contentsScale 与 drawableSize 均需按像素设置，
    // MoltenVK 的 surface currentExtent 会跟随 drawableSize。
    CGFloat scale = view.window ? view.window.backingScaleFactor : 1.0;
    if (scale < 1.0) {
        scale = 1.0;
    }
    layer.contentsScale = scale;
    layer.drawableSize = [view convertSizeToBacking:view.bounds.size];

    view.layer = layer;
    return (__bridge void*)layer;
}

void GetViewClientPixelSize(void* nsView, uint32_t& width, uint32_t& height) {
    if (!nsView) {
        width = 1280;
        height = 720;
        return;
    }

    NSView* view = (__bridge NSView*)nsView;
    CGSize backing = [view convertSizeToBacking:view.bounds.size];
    width = static_cast<uint32_t>(backing.width);
    height = static_cast<uint32_t>(backing.height);
    if (width == 0 || height == 0) {
        width = 1280;
        height = 720;
    }
}

} // namespace nvrhi_lab
