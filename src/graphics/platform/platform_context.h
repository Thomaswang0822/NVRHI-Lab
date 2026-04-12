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

    int getWidth() const { return m_Width; }
    int getHeight() const { return m_Height; }
    virtual const char* getBackendName() const = 0;

    nvrhi::Format getSwapChainFormat() const { return m_SwapChainFormat; }

protected:
    int m_Width = 0;
    int m_Height = 0;
    nvrhi::Format m_SwapChainFormat = nvrhi::Format::RGBA8_UNORM;
};

} // namespace nvrhi_lab
