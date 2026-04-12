#pragma once

#include <nvrhi/nvrhi.h>
#include <memory>
#include <string>

namespace nvrhi_lab {

enum class GraphicsBackend {
    D3D11,
    D3D12,
    Vulkan
};

struct DeviceManagerDesc {
    GraphicsBackend backend = GraphicsBackend::D3D12;
    std::string windowTitle = "NVRHI Lab";
    uint32_t width = 1280;
    uint32_t height = 720;
    bool enableValidation = true;
    bool enableDebugLayer = true;
    bool vsyncEnabled = true;
    uint32_t backBufferCount = 2;
};

class IPlatformContext;

class DeviceManager {
public:
    DeviceManager();
    ~DeviceManager();

    bool Initialize(const DeviceManagerDesc& desc, void* windowHandle);
    void Shutdown();

    bool BeginFrame();
    void Present();

    nvrhi::IDevice* GetDevice() const { return m_Device; }
    nvrhi::ITexture* GetCurrentBackBuffer() const;
    nvrhi::ITexture* GetBackBuffer(uint32_t index) const;
    uint32_t GetCurrentBackBufferIndex() const;
    uint32_t GetBackBufferCount() const { return m_Desc.backBufferCount; }
    uint32_t GetWidth() const { return m_Desc.width; }
    uint32_t GetHeight() const { return m_Desc.height; }
    GraphicsBackend GetBackend() const { return m_Desc.backend; }
    const char* GetBackendName() const;
    nvrhi::Format GetSwapChainFormat() const;
    bool IsInitialized() const { return m_Initialized; }

private:
    DeviceManagerDesc m_Desc;
    void* m_WindowHandle = nullptr;
    bool m_Initialized = false;

    std::unique_ptr<IPlatformContext> m_Platform;
    nvrhi::DeviceHandle m_Device;
};

} // namespace nvrhi_lab
