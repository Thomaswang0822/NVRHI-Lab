#include "device_manager.h"

#include "platform/d3d11_context.h"
#include "platform/d3d12_context.h"
#include "platform/vulkan_context.h"
#include "../utils/logging.h"

namespace nvrhi_lab {

namespace {

class MessageCallback : public nvrhi::IMessageCallback {
public:
    void message(nvrhi::MessageSeverity severity, const char* messageText) override {
        switch (severity) {
            case nvrhi::MessageSeverity::Error:
                LogError(messageText);
                break;
            case nvrhi::MessageSeverity::Warning:
                LogWarning(messageText);
                break;
            case nvrhi::MessageSeverity::Info:
                LogInfo(messageText);
                break;
        }
    }
};

static MessageCallback g_MessageCallback;

void GetWindowClientSize(void* windowHandle, uint32_t& width, uint32_t& height) {
#ifdef WIN32
    HWND hwnd = static_cast<HWND>(windowHandle);
    RECT rect;
    GetClientRect(hwnd, &rect);
    width = static_cast<uint32_t>(rect.right - rect.left);
    height = static_cast<uint32_t>(rect.bottom - rect.top);
#else
    width = 1280;
    height = 720;
#endif
}

} // anonymous namespace

DeviceManager::DeviceManager() = default;

DeviceManager::~DeviceManager() {
    if (m_Initialized) {
        Shutdown();
    }
}

bool DeviceManager::Initialize(const DeviceManagerDesc& desc, void* windowHandle) {
    if (m_Initialized) {
        return true;
    }

    m_Desc = desc;
    m_WindowHandle = windowHandle;

    GetWindowClientSize(windowHandle, m_Desc.width, m_Desc.height);

    switch (m_Desc.backend) {
        case GraphicsBackend::D3D12:
            m_Platform = std::make_unique<D3D12Context>();
            break;
        case GraphicsBackend::D3D11:
            m_Platform = std::make_unique<D3D11Context>();
            break;
        case GraphicsBackend::Vulkan:
            m_Platform = std::make_unique<VulkanContext>();
            break;
    }

    m_Device = m_Platform->createNvrhiDevice(&g_MessageCallback, m_Desc.enableValidation);
    if (!m_Device) {
        LogError("Failed to create NVRHI device");
        m_Platform.reset();
        return false;
    }

    m_Platform->createSwapChain(windowHandle, m_Desc.width, m_Desc.height, m_Desc.backBufferCount);

    m_Initialized = true;
    return true;
}

void DeviceManager::Shutdown() {
    if (!m_Initialized) {
        return;
    }

    // Ensure GPU has finished with any in-flight work before releasing resources.
    if (m_Platform) {
        m_Platform->waitForIdle();
    }

    // Release NVRHI device and platform objects after the GPU is idle to avoid
    // final-releasing D3D12 resources while they are still referenced by the GPU.
    m_Device = nullptr;
    m_Platform.reset();

    m_Initialized = false;
    m_WindowHandle = nullptr;
}

bool DeviceManager::BeginFrame() {
    if (!m_Initialized) {
        return false;
    }

    m_Platform->waitForIdle();
    return true;
}

void DeviceManager::Present() {
    if (!m_Initialized) {
        return;
    }

    m_Platform->present(m_Desc.vsyncEnabled);
}

nvrhi::ITexture* DeviceManager::GetCurrentBackBuffer() const {
    if (!m_Initialized) {
        return nullptr;
    }
    return m_Platform->getBackBuffer(m_Platform->getCurrentBackBufferIndex());
}

nvrhi::ITexture* DeviceManager::GetBackBuffer(uint32_t index) const {
    if (!m_Initialized) {
        return nullptr;
    }
    return m_Platform->getBackBuffer(index);
}

uint32_t DeviceManager::GetCurrentBackBufferIndex() const {
    if (!m_Initialized) {
        return 0;
    }
    return static_cast<uint32_t>(m_Platform->getCurrentBackBufferIndex());
}

const char* DeviceManager::GetBackendName() const {
    if (!m_Platform) {
        return "None";
    }
    return m_Platform->getBackendName();
}

nvrhi::Format DeviceManager::GetSwapChainFormat() const {
    if (!m_Platform) {
        return nvrhi::Format::RGBA8_UNORM;
    }
    return m_Platform->getSwapChainFormat();
}

} // namespace nvrhi_lab
