#include "d3d12_context.h"

#include "../../utils/logging.h"

#include <nvrhi/validation.h>
#include <d3d12sdklayers.h>

using Microsoft::WRL::ComPtr;

namespace nvrhi_lab {

D3D12Context::D3D12Context() = default;

D3D12Context::~D3D12Context() {
    if (m_FenceEvent) {
        CloseHandle(m_FenceEvent);
    }
}

nvrhi::DeviceHandle D3D12Context::createNvrhiDevice(nvrhi::IMessageCallback* messageCallback, bool enableValidation) {
    UINT dxgiFactoryFlags = 0;
#if defined(_DEBUG)
    ComPtr<ID3D12Debug> debugController;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
        debugController->EnableDebugLayer();
        dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
    }
#endif

    ComPtr<IDXGIFactory6> factory;
    HRESULT hr = CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory));
    if (FAILED(hr)) {
        LogError("D3D12: Failed to create DXGI factory");
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
        hr = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_Device));
        if (SUCCEEDED(hr)) {
            break;
        }
    }
    if (!m_Device) {
        LogError("D3D12: Failed to create device");
        return nullptr;
    }

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    hr = m_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_CommandQueue));
    if (FAILED(hr)) {
        LogError("D3D12: Failed to create command queue");
        return nullptr;
    }

    hr = m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence));
    if (FAILED(hr)) {
        LogError("D3D12: Failed to create fence");
        return nullptr;
    }

    m_FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (!m_FenceEvent) {
        LogError("D3D12: Failed to create fence event");
        return nullptr;
    }

    nvrhi::d3d12::DeviceDesc nvrhiDesc;
    nvrhiDesc.pDevice = m_Device.Get();
    nvrhiDesc.pGraphicsCommandQueue = m_CommandQueue.Get();
    nvrhiDesc.errorCB = messageCallback;

    m_NvrhiDevice = nvrhi::d3d12::createDevice(nvrhiDesc);
    if (!m_NvrhiDevice) {
        LogError("D3D12: Failed to create NVRHI device");
        return nullptr;
    }

    if (enableValidation) {
        return nvrhi::validation::createValidationLayer(m_NvrhiDevice);
    }

    return m_NvrhiDevice;
}

void D3D12Context::createSwapChain(void* windowHandle, int width, int height, int backBufferCount) {
    if (!m_Factory || !m_CommandQueue) {
        LogError("D3D12: Cannot create swap chain before device");
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
        m_CommandQueue.Get(),
        hwnd,
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain1
    );
    if (FAILED(hr)) {
        LogError("D3D12: Failed to create swap chain");
        return;
    }

    hr = swapChain1.As(&m_SwapChain);
    if (FAILED(hr)) {
        LogError("D3D12: Failed to get swap chain4");
        return;
    }

    m_Width = width;
    m_Height = height;
    m_BackBufferCount = backBufferCount;

    m_SwapChainBuffers.resize(backBufferCount);
    for (int i = 0; i < backBufferCount; ++i) {
        ComPtr<ID3D12Resource> backBuffer;
        hr = m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer));
        if (FAILED(hr)) {
            LogError("D3D12: Failed to get back buffer " + std::to_string(i));
            continue;
        }

        nvrhi::TextureDesc texDesc;
        texDesc.width = static_cast<uint32_t>(width);
        texDesc.height = static_cast<uint32_t>(height);
        texDesc.format = m_SwapChainFormat;
        texDesc.isRenderTarget = true;
        texDesc.initialState = nvrhi::ResourceStates::Present;
        texDesc.keepInitialState = true;
        texDesc.debugName = "BackBuffer_" + std::to_string(i);

        // NVRHI takes ownership of the native resource handle; keep a local
        // reference to the ID3D12Resource to ensure the resource stays alive
        // until NVRHI has registered it. This helps avoid a race where the
        // back buffer could be released while GPU work still references it.
        // Note: createHandleForNativeTexture should internally add a reference
        // if it needs to keep the resource alive. We defensively AddRef here
        // and Release after creating the handle to ensure correctness across
        // different nvrhi implementations.
        backBuffer->AddRef();
        m_SwapChainBuffers[i] = m_NvrhiDevice->createHandleForNativeTexture(
            nvrhi::ObjectTypes::D3D12_Resource,
            backBuffer.Get(),
            texDesc
        );
        // Release our local extra ref; NVRHI is expected to keep its own ref
        // if required.
        backBuffer->Release();
    }
}

nvrhi::ITexture* D3D12Context::getBackBuffer(size_t index) {
    if (index >= m_SwapChainBuffers.size()) {
        return nullptr;
    }
    return m_SwapChainBuffers[index];
}

size_t D3D12Context::getCurrentBackBufferIndex() {
    if (!m_SwapChain) {
        return 0;
    }
    return m_SwapChain->GetCurrentBackBufferIndex();
}

void D3D12Context::present(bool vsync) {
    if (m_SwapChain) {
        m_SwapChain->Present(vsync ? 1 : 0, 0);
    }
}

void D3D12Context::waitForIdle() {
    if (!m_CommandQueue || !m_Fence || !m_FenceEvent) {
        return;
    }

    const uint64_t fenceToWait = ++m_FenceValue;
    m_CommandQueue->Signal(m_Fence.Get(), fenceToWait);

    if (m_Fence->GetCompletedValue() < fenceToWait) {
        m_Fence->SetEventOnCompletion(fenceToWait, m_FenceEvent);
        WaitForSingleObject(m_FenceEvent, INFINITE);
    }
}

} // namespace nvrhi_lab
