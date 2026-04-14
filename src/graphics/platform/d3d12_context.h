#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <dxgi1_6.h>
#include <d3d12.h>
#include <wrl/client.h>

#include <nvrhi/d3d12.h>

#include "platform_context.h"

namespace nvrhi_lab {

using Microsoft::WRL::ComPtr;

class D3D12Context : public IPlatformContext {
public:
    D3D12Context();
    ~D3D12Context();

    nvrhi::DeviceHandle createNvrhiDevice(nvrhi::IMessageCallback* messageCallback, bool enableValidation) override;

    void createSwapChain(void* windowHandle, int width, int height, int backBufferCount) override;
    nvrhi::ITexture* getBackBuffer(size_t index) override;
    size_t getCurrentBackBufferIndex() override;
    void present(bool vsync) override;

    void waitForIdle() override;

    const char* getBackendName() const override { return "D3D12"; }

private:
    int m_BackBufferCount = 0;

    ComPtr<IDXGIFactory6> m_Factory;
    ComPtr<ID3D12Device> m_Device;
    ComPtr<ID3D12CommandQueue> m_CommandQueue;
    ComPtr<IDXGISwapChain4> m_SwapChain;
    ComPtr<ID3D12Fence> m_Fence;
    HANDLE m_FenceEvent = nullptr;
    uint64_t m_FenceValue = 0;

    nvrhi::DeviceHandle m_NvrhiDevice;
    std::vector<nvrhi::TextureHandle> m_SwapChainBuffers;
};

} // namespace nvrhi_lab
