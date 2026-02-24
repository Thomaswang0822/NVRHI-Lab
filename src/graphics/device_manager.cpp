#include "device_manager.h"

#include <nvrhi/d3d11.h>
#include <nvrhi/d3d12.h>
#include <nvrhi/validation.h>

#include <d3d11.h>
#include <d3d12.h>
#include <dxgi1_6.h>

#include <stdexcept>
#include <sstream>

#ifdef WIN32
#include <windows.h>
#endif

namespace nvrhi_lab {

namespace {

// Helper to convert NVRHI format to DXGI format
DXGI_FORMAT ConvertFormat(nvrhi::Format format) {
    return nvrhi::d3d12::convertFormat(format);
}

// Helper function to get the window client size
void GetWindowClientSize(void* windowHandle, uint32_t& width, uint32_t& height) {
    HWND hwnd = static_cast<HWND>(windowHandle);
    RECT rect;
    GetClientRect(hwnd, &rect);
    width = static_cast<uint32_t>(rect.right - rect.left);
    height = static_cast<uint32_t>(rect.bottom - rect.top);
}

} // anonymous namespace

DeviceManager::DeviceManager() = default;

DeviceManager::~DeviceManager() {
    Shutdown();
}

bool DeviceManager::Initialize(const DeviceManagerDesc& desc, void* windowHandle) {
    if (m_Initialized) {
        return true;
    }

    m_Desc = desc;
    m_WindowHandle = windowHandle;

    // Update dimensions from actual window size
    GetWindowClientSize(windowHandle, m_Desc.width, m_Desc.height);

    bool success = false;
    switch (m_Desc.backend) {
        case GraphicsBackend::D3D11:
            success = CreateD3D11Device() && CreateD3D11SwapChain();
            break;
        case GraphicsBackend::D3D12:
            success = CreateD3D12Device() && CreateD3D12SwapChain();
            break;
        case GraphicsBackend::Vulkan:
            // Vulkan not implemented yet
            success = false;
            break;
    }

    if (!success) {
        Shutdown();
        return false;
    }

    m_Initialized = true;
    return true;
}

void DeviceManager::Shutdown() {
    // Release NVRHI objects first
    m_SwapChainBuffers.clear();
    m_ValidationLayer = nullptr;
    m_Device = nullptr;

    // Cleanup backend-specific resources
    switch (m_Desc.backend) {
        case GraphicsBackend::D3D11:
            DestroyD3D11();
            break;
        case GraphicsBackend::D3D12:
            DestroyD3D12();
            break;
        case GraphicsBackend::Vulkan:
            DestroyVulkan();
            break;
    }

    m_Initialized = false;
    m_WindowHandle = nullptr;
}

bool DeviceManager::BeginFrame() {
    if (!m_Initialized) {
        return false;
    }

    // Wait for the previous frame on this backbuffer to complete (D3D12)
    if (m_Desc.backend == GraphicsBackend::D3D12 && m_D3D12Fence) {
        ID3D12Fence* fence = static_cast<ID3D12Fence*>(m_D3D12Fence);
        uint64_t fenceValue = m_D3D12FenceValue - (m_Desc.backBufferCount - 1);
        if (fenceValue > 0 && fence->GetCompletedValue() < fenceValue) {
            HANDLE event = static_cast<HANDLE>(m_D3D12FenceEvent);
            fence->SetEventOnCompletion(fenceValue, event);
            WaitForSingleObject(event, INFINITE);
        }
    }

    return true;
}

void DeviceManager::Present() {
    if (!m_Initialized) {
        return;
    }

    switch (m_Desc.backend) {
        case GraphicsBackend::D3D11: {
            IDXGISwapChain* swapChain = static_cast<IDXGISwapChain*>(m_D3D11SwapChain);
            swapChain->Present(m_Desc.vsyncEnabled ? 1 : 0, 0);
            break;
        }
        case GraphicsBackend::D3D12: {
            IDXGISwapChain3* swapChain = static_cast<IDXGISwapChain3*>(m_D3D12SwapChain);
            
            // Signal the fence
            ID3D12CommandQueue* commandQueue = static_cast<ID3D12CommandQueue*>(m_D3D12CommandQueue);
            ID3D12Fence* fence = static_cast<ID3D12Fence*>(m_D3D12Fence);
            commandQueue->Signal(fence, m_D3D12FenceValue);
            
            swapChain->Present(m_Desc.vsyncEnabled ? 1 : 0, 0);
            m_CurrentBackBufferIndex = swapChain->GetCurrentBackBufferIndex();
            m_D3D12FenceValue++;
            break;
        }
        case GraphicsBackend::Vulkan:
            // Not implemented
            break;
    }
}

nvrhi::ITexture* DeviceManager::GetCurrentBackBuffer() const {
    if (m_SwapChainBuffers.empty()) {
        return nullptr;
    }
    return m_SwapChainBuffers[m_CurrentBackBufferIndex];
}

nvrhi::ITexture* DeviceManager::GetBackBuffer(uint32_t index) const {
    if (index >= m_SwapChainBuffers.size()) {
        return nullptr;
    }
    return m_SwapChainBuffers[index];
}

const char* DeviceManager::GetBackendName() const {
    switch (m_Desc.backend) {
        case GraphicsBackend::D3D11:
            return "D3D11";
        case GraphicsBackend::D3D12:
            return "D3D12";
        case GraphicsBackend::Vulkan:
            return "Vulkan";
        default:
            return "Unknown";
    }
}

// D3D11 Implementation
bool DeviceManager::CreateD3D11Device() {
    // Create D3D11 device
    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
    };

    UINT createDeviceFlags = 0;
    if (m_Desc.enableDebugLayer) {
        createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    }

    D3D_FEATURE_LEVEL selectedFeatureLevel;
    ID3D11Device* device = nullptr;
    ID3D11DeviceContext* context = nullptr;

    HRESULT hr = D3D11CreateDevice(
        nullptr,                    // Adapter
        D3D_DRIVER_TYPE_HARDWARE,   // Driver type
        nullptr,                    // Software
        createDeviceFlags,
        featureLevels,
        ARRAYSIZE(featureLevels),
        D3D11_SDK_VERSION,
        &device,
        &selectedFeatureLevel,
        &context
    );

    if (FAILED(hr)) {
        return false;
    }

    m_D3D11Device = device;
    m_D3D11Context = context;

    // Create NVRHI device
    nvrhi::d3d11::DeviceDesc nvrhiDesc;
    nvrhiDesc.context = context;
    // nvrhiDesc.messageCallback = ...; // Could add error callback here

    nvrhi::DeviceHandle nvrhiDevice = nvrhi::d3d11::createDevice(nvrhiDesc);
    if (!nvrhiDevice) {
        return false;
    }

    // Wrap with validation layer if enabled
    if (m_Desc.enableValidation) {
        m_ValidationLayer = nvrhi::validation::createValidationLayer(nvrhiDevice);
        m_Device = m_ValidationLayer;
    } else {
        m_Device = nvrhiDevice;
    }

    return true;
}

bool DeviceManager::CreateD3D11SwapChain() {
    ID3D11Device* device = static_cast<ID3D11Device*>(m_D3D11Device);
    HWND hwnd = static_cast<HWND>(m_WindowHandle);

    // Get DXGI factory from device
    IDXGIDevice* dxgiDevice = nullptr;
    HRESULT hr = device->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);
    if (FAILED(hr)) {
        return false;
    }

    IDXGIAdapter* dxgiAdapter = nullptr;
    hr = dxgiDevice->GetAdapter(&dxgiAdapter);
    dxgiDevice->Release();
    if (FAILED(hr)) {
        return false;
    }

    IDXGIFactory* dxgiFactory = nullptr;
    hr = dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&dxgiFactory);
    dxgiAdapter->Release();
    if (FAILED(hr)) {
        return false;
    }

    // Create swap chain
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    swapChainDesc.BufferCount = m_Desc.backBufferCount;
    swapChainDesc.BufferDesc.Width = m_Desc.width;
    swapChainDesc.BufferDesc.Height = m_Desc.height;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 0;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = hwnd;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.Windowed = TRUE;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swapChainDesc.Flags = 0;

    IDXGISwapChain* swapChain = nullptr;
    hr = dxgiFactory->CreateSwapChain(device, &swapChainDesc, &swapChain);
    dxgiFactory->Release();
    if (FAILED(hr)) {
        return false;
    }

    m_D3D11SwapChain = swapChain;

    // Create NVRHI textures for swap chain buffers
    m_SwapChainBuffers.resize(m_Desc.backBufferCount);
    for (uint32_t i = 0; i < m_Desc.backBufferCount; ++i) {
        ID3D11Texture2D* backBuffer = nullptr;
        hr = swapChain->GetBuffer(i, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
        if (FAILED(hr)) {
            return false;
        }

        // Create texture description
        nvrhi::TextureDesc textureDesc;
        textureDesc.width = m_Desc.width;
        textureDesc.height = m_Desc.height;
        textureDesc.format = nvrhi::Format::RGBA8_UNORM;
        textureDesc.isRenderTarget = true;
        textureDesc.initialState = nvrhi::ResourceStates::Present;
        textureDesc.debugName = "BackBuffer_" + std::to_string(i);

        // Create texture from native handle
        m_SwapChainBuffers[i] = m_Device->createHandleForNativeTexture(
            nvrhi::ObjectTypes::D3D11_Resource,
            backBuffer,
            textureDesc
        );

        backBuffer->Release();
    }

    return true;
}

void DeviceManager::DestroyD3D11() {
    if (m_D3D11SwapChain) {
        static_cast<IDXGISwapChain*>(m_D3D11SwapChain)->Release();
        m_D3D11SwapChain = nullptr;
    }
    if (m_D3D11Context) {
        static_cast<ID3D11DeviceContext*>(m_D3D11Context)->Release();
        m_D3D11Context = nullptr;
    }
    if (m_D3D11Device) {
        static_cast<ID3D11Device*>(m_D3D11Device)->Release();
        m_D3D11Device = nullptr;
    }
}

// D3D12 Implementation
bool DeviceManager::CreateD3D12Device() {
    // Enable debug layer if requested
    if (m_Desc.enableDebugLayer) {
        ID3D12Debug* debugController = nullptr;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
            debugController->EnableDebugLayer();
            debugController->Release();
        }
    }

    // Create D3D12 device
    ID3D12Device* device = nullptr;
    HRESULT hr = D3D12CreateDevice(
        nullptr,                    // Adapter
        D3D_FEATURE_LEVEL_11_0,
        IID_PPV_ARGS(&device)
    );

    if (FAILED(hr)) {
        return false;
    }

    m_D3D12Device = device;

    // Create command queue
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

    ID3D12CommandQueue* commandQueue = nullptr;
    hr = device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue));
    if (FAILED(hr)) {
        return false;
    }

    m_D3D12CommandQueue = commandQueue;

    // Create fence for synchronization
    ID3D12Fence* fence = nullptr;
    hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
    if (FAILED(hr)) {
        return false;
    }

    m_D3D12Fence = fence;
    m_D3D12FenceValue = 1;

    // Create fence event
    m_D3D12FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (!m_D3D12FenceEvent) {
        return false;
    }

    // Create NVRHI device
    nvrhi::d3d12::DeviceDesc nvrhiDesc;
    nvrhiDesc.pDevice = device;
    nvrhiDesc.pGraphicsCommandQueue = commandQueue;
    // nvrhiDesc.errorCB = ...; // Could add error callback here

    nvrhi::DeviceHandle nvrhiDevice = nvrhi::d3d12::createDevice(nvrhiDesc);
    if (!nvrhiDevice) {
        return false;
    }

    // Wrap with validation layer if enabled
    if (m_Desc.enableValidation) {
        m_ValidationLayer = nvrhi::validation::createValidationLayer(nvrhiDevice);
        m_Device = m_ValidationLayer;
    } else {
        m_Device = nvrhiDevice;
    }

    return true;
}

bool DeviceManager::CreateD3D12SwapChain() {
    ID3D12Device* device = static_cast<ID3D12Device*>(m_D3D12Device);
    ID3D12CommandQueue* commandQueue = static_cast<ID3D12CommandQueue*>(m_D3D12CommandQueue);
    HWND hwnd = static_cast<HWND>(m_WindowHandle);

    // Get DXGI factory
    IDXGIFactory4* dxgiFactory = nullptr;
    HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));
    if (FAILED(hr)) {
        return false;
    }

    // Create swap chain
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width = m_Desc.width;
    swapChainDesc.Height = m_Desc.height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = m_Desc.backBufferCount;
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
    swapChainDesc.Flags = 0;

    IDXGISwapChain1* swapChain1 = nullptr;
    hr = dxgiFactory->CreateSwapChainForHwnd(
        commandQueue,
        hwnd,
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain1
    );

    if (FAILED(hr)) {
        dxgiFactory->Release();
        return false;
    }

    // Query for IDXGISwapChain3 (needed for GetCurrentBackBufferIndex)
    IDXGISwapChain3* swapChain3 = nullptr;
    hr = swapChain1->QueryInterface(IID_PPV_ARGS(&swapChain3));
    swapChain1->Release();
    dxgiFactory->Release();

    if (FAILED(hr)) {
        return false;
    }

    m_D3D12SwapChain = swapChain3;

    // Create NVRHI textures for swap chain buffers
    m_SwapChainBuffers.resize(m_Desc.backBufferCount);
    for (uint32_t i = 0; i < m_Desc.backBufferCount; ++i) {
        ID3D12Resource* backBuffer = nullptr;
        hr = swapChain3->GetBuffer(i, IID_PPV_ARGS(&backBuffer));
        if (FAILED(hr)) {
            return false;
        }

        // Create texture description
        nvrhi::TextureDesc textureDesc;
        textureDesc.width = m_Desc.width;
        textureDesc.height = m_Desc.height;
        textureDesc.format = nvrhi::Format::RGBA8_UNORM;
        textureDesc.isRenderTarget = true;
        textureDesc.initialState = nvrhi::ResourceStates::Present;
        textureDesc.debugName = "BackBuffer_" + std::to_string(i);

        // Create texture from native handle
        m_SwapChainBuffers[i] = m_Device->createHandleForNativeTexture(
            nvrhi::ObjectTypes::D3D12_Resource,
            backBuffer,
            textureDesc
        );

        backBuffer->Release();
    }

    m_CurrentBackBufferIndex = swapChain3->GetCurrentBackBufferIndex();

    return true;
}

void DeviceManager::DestroyD3D12() {
    if (m_D3D12FenceEvent) {
        CloseHandle(static_cast<HANDLE>(m_D3D12FenceEvent));
        m_D3D12FenceEvent = nullptr;
    }
    if (m_D3D12Fence) {
        static_cast<ID3D12Fence*>(m_D3D12Fence)->Release();
        m_D3D12Fence = nullptr;
    }
    if (m_D3D12SwapChain) {
        static_cast<IDXGISwapChain3*>(m_D3D12SwapChain)->Release();
        m_D3D12SwapChain = nullptr;
    }
    if (m_D3D12CommandQueue) {
        static_cast<ID3D12CommandQueue*>(m_D3D12CommandQueue)->Release();
        m_D3D12CommandQueue = nullptr;
    }
    if (m_D3D12Device) {
        static_cast<ID3D12Device*>(m_D3D12Device)->Release();
        m_D3D12Device = nullptr;
    }
}

// Vulkan Implementation (stub)
bool DeviceManager::CreateVulkanDevice() {
    // Not implemented yet
    return false;
}

bool DeviceManager::CreateVulkanSwapChain() {
    // Not implemented yet
    return false;
}

void DeviceManager::DestroyVulkan() {
    // Not implemented yet
}

} // namespace nvrhi_lab
