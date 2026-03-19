#pragma once

#include <nvrhi/nvrhi.h>
#include <memory>
#include <string>

// Forward declarations for platform-specific handles
#ifdef WIN32
struct HWND__;
typedef HWND__* HWND;
#endif

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

class DeviceManager {
public:
    DeviceManager();
    ~DeviceManager();

    // Initialize the device manager with a window handle
    bool Initialize(const DeviceManagerDesc& desc, void* windowHandle);

    // Shutdown and cleanup
    void Shutdown();

    // Frame management
    bool BeginFrame();
    void Present();

    // Getters
    nvrhi::IDevice* GetDevice() const { return m_Device; }
    nvrhi::ITexture* GetCurrentBackBuffer() const;
    nvrhi::ITexture* GetBackBuffer(uint32_t index) const;
    uint32_t GetCurrentBackBufferIndex() const { return m_CurrentBackBufferIndex; }
    uint32_t GetBackBufferCount() const { return m_Desc.backBufferCount; }
    uint32_t GetWidth() const { return m_Desc.width; }
    uint32_t GetHeight() const { return m_Desc.height; }
    GraphicsBackend GetBackend() const { return m_Desc.backend; }
    const char* GetBackendName() const;
    bool IsInitialized() const { return m_Initialized; }

private:
    DeviceManagerDesc m_Desc;
    void* m_WindowHandle = nullptr;
    bool m_Initialized = false;
    uint32_t m_CurrentBackBufferIndex = 0;

    // NVRHI objects
    nvrhi::DeviceHandle m_Device;
    nvrhi::DeviceHandle m_ValidationLayer;
    std::vector<nvrhi::TextureHandle> m_SwapChainBuffers;

    // Backend-specific objects (stored as void* to avoid exposing platform headers)
    void* m_D3D11Device = nullptr;
    void* m_D3D11Context = nullptr;
    void* m_D3D11SwapChain = nullptr;
    
    void* m_D3D12Device = nullptr;
    void* m_D3D12CommandQueue = nullptr;
    void* m_D3D12SwapChain = nullptr;
    void* m_D3D12Fence = nullptr;
    uint64_t m_D3D12FenceValue = 0;
    void* m_D3D12FenceEvent = nullptr;

    // Backend-specific creation methods
    bool CreateD3D11Device();
    bool CreateD3D12Device();
    bool CreateVulkanDevice();
    
    bool CreateD3D11SwapChain();
    bool CreateD3D12SwapChain();
    bool CreateVulkanSwapChain();

    void DestroyD3D11();
    void DestroyD3D12();
    void DestroyVulkan();

    void WaitForGpuIdle();
};

} // namespace nvrhi_lab
