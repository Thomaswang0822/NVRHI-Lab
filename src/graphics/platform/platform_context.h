#pragma once

#include <nvrhi/nvrhi.h>
#include <cstddef>

namespace nvrhi_lab {

class IPlatformContext {
public:
    virtual ~IPlatformContext() = default;

    virtual nvrhi::DeviceHandle createNvrhiDevice(nvrhi::IMessageCallback* messageCallback, bool enableValidation) = 0;

    virtual void createSwapChain(void* windowHandle, int width, int height, int backBufferCount) = 0;
    virtual nvrhi::ITexture* getBackBuffer(size_t index) = 0;
    virtual size_t getCurrentBackBufferIndex() = 0;
    virtual void present(bool vsync) = 0;

    virtual void waitForIdle() = 0;

    virtual int getWidth() const = 0;
    virtual int getHeight() const = 0;
    virtual const char* getBackendName() const = 0;
};

} // namespace nvrhi_lab
