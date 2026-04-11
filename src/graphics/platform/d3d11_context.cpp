#include "d3d11_context.h"

#include "../../utils/logging.h"

#include <nvrhi/validation.h>

using Microsoft::WRL::ComPtr;

namespace nvrhi_lab {

D3D11Context::D3D11Context() = default;

D3D11Context::~D3D11Context() = default;

nvrhi::DeviceHandle D3D11Context::createNvrhiDevice(nvrhi::IMessageCallback* messageCallback, bool enableValidation) {
    UINT createDeviceFlags = 0;
#if defined(_DEBUG)
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    ComPtr<IDXGIFactory6> factory;
    HRESULT hr = CreateDXGIFactory2(0, IID_PPV_ARGS(&factory));
    if (FAILED(hr)) {
        LogError("D3D11: Failed to create DXGI factory");
        return nullptr;
    }
    m_Factory = factory;

    ComPtr<IDXGIAdapter1> adapter;
    for (UINT i = 0; factory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND; ++i) {
        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);
        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
            continue;
        }
        
        hr = D3D11CreateDevice(
            adapter.Get(),
            D3D_DRIVER_TYPE_UNKNOWN,
            nullptr,
            createDeviceFlags,
            nullptr,
            0,
            D3D11_SDK_VERSION,
            &m_Device,
            nullptr,
            &m_ImmediateContext
        );
        if (SUCCEEDED(hr)) {
            break;
        }
    }
    
    if (!m_Device) {
        LogError("D3D11: Failed to create device");
        return nullptr;
    }

    nvrhi::d3d11::DeviceDesc nvrhiDesc;
    nvrhiDesc.messageCallback = messageCallback;
    nvrhiDesc.context = m_ImmediateContext.Get();

    m_NvrhiDevice = nvrhi::d3d11::createDevice(nvrhiDesc);
    if (!m_NvrhiDevice) {
        LogError("D3D11: Failed to create NVRHI device");
        return nullptr;
    }

    if (enableValidation) {
        return nvrhi::validation::createValidationLayer(m_NvrhiDevice);
    }

    return m_NvrhiDevice;
}

void D3D11Context::createSwapChain(void* windowHandle, int width, int height, int backBufferCount) {
    if (!m_Factory || !m_Device) {
        LogError("D3D11: Cannot create swap chain before device");
        return;
    }

    HWND hwnd = static_cast<HWND>(windowHandle);

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width = static_cast<UINT>(width);
    swapChainDesc.Height = static_cast<UINT>(height);
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = static_cast<UINT>(backBufferCount);
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
    swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

    ComPtr<IDXGISwapChain1> swapChain1;
    HRESULT hr = m_Factory->CreateSwapChainForHwnd(
        m_Device.Get(),
        hwnd,
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain1
    );
    if (FAILED(hr)) {
        LogError("D3D11: Failed to create swap chain");
        return;
    }

    hr = swapChain1.As(&m_SwapChain);
    if (FAILED(hr)) {
        LogError("D3D11: Failed to get swap chain4");
        return;
    }

    m_Width = width;
    m_Height = height;
    m_BackBufferCount = backBufferCount;

    m_SwapChainBuffers.resize(backBufferCount);
    for (int i = 0; i < backBufferCount; ++i) {
        ComPtr<ID3D11Texture2D> backBuffer;
        hr = m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer));
        if (FAILED(hr)) {
            LogError("D3D11: Failed to get back buffer " + std::to_string(i));
            continue;
        }

        nvrhi::TextureDesc texDesc;
        texDesc.width = static_cast<uint32_t>(width);
        texDesc.height = static_cast<uint32_t>(height);
        texDesc.format = nvrhi::Format::RGBA8_UNORM;
        texDesc.isRenderTarget = true;
        texDesc.initialState = nvrhi::ResourceStates::Present;
        texDesc.debugName = "BackBuffer_" + std::to_string(i);

        m_SwapChainBuffers[i] = m_NvrhiDevice->createHandleForNativeTexture(
            nvrhi::ObjectTypes::D3D11_Resource,
            backBuffer.Get(),
            texDesc
        );
    }
}

nvrhi::ITexture* D3D11Context::getBackBuffer(size_t index) {
    if (index >= m_SwapChainBuffers.size()) {
        return nullptr;
    }
    return m_SwapChainBuffers[index];
}

size_t D3D11Context::getCurrentBackBufferIndex() {
    if (!m_SwapChain) {
        return 0;
    }
    return m_SwapChain->GetCurrentBackBufferIndex();
}

void D3D11Context::present(bool vsync) {
    if (m_SwapChain) {
        m_SwapChain->Present(vsync ? 1 : 0, 0);
    }
}

void D3D11Context::waitForIdle() {
    if (m_ImmediateContext) {
        m_ImmediateContext->Flush();
    }
}

} // namespace nvrhi_lab
