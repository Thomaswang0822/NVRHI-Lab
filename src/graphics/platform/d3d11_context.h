#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <dxgi1_6.h>
#include <d3d11.h>
#include <wrl/client.h>

#include <nvrhi/d3d11.h>

#include "platform_context.h"

namespace nvrhi_lab {

using Microsoft::WRL::ComPtr;

class D3D11Context : public IPlatformContext {
public:
    D3D11Context();
    ~D3D11Context();

    nvrhi::DeviceHandle createNvrhiDevice(nvrhi::IMessageCallback* messageCallback, bool enableValidation) override;

    void createSwapChain(void* windowHandle, int width, int height, int backBufferCount) override;
    nvrhi::ITexture* getBackBuffer(size_t index) override;
    size_t getCurrentBackBufferIndex() override;
    void present(bool vsync) override;

    void waitForIdle() override;

    int getWidth() const override { return m_Width; }
    int getHeight() const override { return m_Height; }
    const char* getBackendName() const override { return "D3D11"; }

private:
    int m_Width = 0;
    int m_Height = 0;
    int m_BackBufferCount = 0;

    ComPtr<IDXGIFactory6> m_Factory;
    ComPtr<ID3D11Device> m_Device;
    ComPtr<ID3D11DeviceContext> m_ImmediateContext;
    ComPtr<IDXGISwapChain4> m_SwapChain;

    nvrhi::DeviceHandle m_NvrhiDevice;
    std::vector<nvrhi::TextureHandle> m_SwapChainBuffers;
};

} // namespace nvrhi_lab
