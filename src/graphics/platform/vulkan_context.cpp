#include "vulkan_context.h"

#include "../../utils/logging.h"

#include <nvrhi/validation.h>

#ifdef __APPLE__
#include "macos_surface.h"
#endif

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace nvrhi_lab {

namespace {

PFN_vkGetInstanceProcAddr initVulkanDispatcherGlobal() {
#if VK_HEADER_VERSION >= 301
    static vk::detail::DynamicLoader dl;
#else
    static vk::DynamicLoader dl;
#endif
    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr =
        dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
    VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);
    return vkGetInstanceProcAddr;
}

}

VulkanContext::VulkanContext() = default;

VulkanContext::~VulkanContext() {
    m_SwapChainBuffers.clear();
    m_NvrhiDevice.Reset();

    if (m_Device) {
        m_Device.waitIdle();
    }

    if (m_ImageAvailableSemaphore) {
        m_Device.destroySemaphore(m_ImageAvailableSemaphore);
    }
    if (m_RenderFinishedSemaphore) {
        m_Device.destroySemaphore(m_RenderFinishedSemaphore);
    }
    if (m_InFlightFence) {
        m_Device.destroyFence(m_InFlightFence);
    }
    if (m_SwapChain) {
        m_Device.destroySwapchainKHR(m_SwapChain);
    }
    if (m_Surface) {
        m_Instance.destroySurfaceKHR(m_Surface);
    }
    if (m_Device) {
        m_Device.destroy();
    }
    if (m_Instance) {
        m_Instance.destroy();
    }
}

nvrhi::DeviceHandle VulkanContext::createNvrhiDevice(nvrhi::IMessageCallback* messageCallback, bool enableValidation) {
    initVulkanDispatcherGlobal();

    vk::ApplicationInfo appInfo;
    appInfo.pApplicationName = "NVRHI Lab";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "NVRHI Lab";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    std::vector<const char*> instanceExtensions = {
        VK_KHR_SURFACE_EXTENSION_NAME
    };
#ifdef _WIN32
    instanceExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif
#ifdef __APPLE__
    instanceExtensions.push_back(VK_EXT_METAL_SURFACE_EXTENSION_NAME);
    // MoltenVK 需要开启 portability 枚举才会被 vkEnumeratePhysicalDevices 返回
    instanceExtensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#endif

    std::vector<const char*> instanceLayers;

    // clang 下 Debug 构建不会自动定义 _DEBUG，用 NDEBUG 判断（MSVC Debug 两者都满足）
#if defined(_DEBUG) || !defined(NDEBUG)
    auto availableLayers = vk::enumerateInstanceLayerProperties();
    for (const auto& layer : availableLayers) {
        if (strcmp(layer.layerName, "VK_LAYER_KHRONOS_validation") == 0) {
            instanceLayers.push_back("VK_LAYER_KHRONOS_validation");
            break;
        }
    }
#endif

    vk::InstanceCreateInfo instanceCreateInfo;
    instanceCreateInfo.pApplicationInfo = &appInfo;
    instanceCreateInfo.setPEnabledExtensionNames(instanceExtensions);
    instanceCreateInfo.setPEnabledLayerNames(instanceLayers);
#ifdef __APPLE__
    instanceCreateInfo.flags |= vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;
#endif

    try {
        m_Instance = vk::createInstance(instanceCreateInfo);
    } catch (const vk::SystemError& e) {
        LogError("Vulkan: Failed to create instance: " + std::string(e.what()));
        return nullptr;
    }

    VULKAN_HPP_DEFAULT_DISPATCHER.init(m_Instance);

    auto devices = m_Instance.enumeratePhysicalDevices();
    if (devices.empty()) {
        LogError("Vulkan: No physical devices found");
        return nullptr;
    }

    m_PhysicalDevice = nullptr;
    int selectedQueueFamilyIndex = -1;

    for (const auto& device : devices) {
        auto props = device.getProperties();

        auto queueFamilies = device.getQueueFamilyProperties();

        int graphicsQueueFamily = -1;
        for (uint32_t i = 0; i < queueFamilies.size(); ++i) {
            if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics) {
                graphicsQueueFamily = static_cast<int>(i);
                break;
            }
        }

        if (graphicsQueueFamily >= 0) {
            if (!m_PhysicalDevice || props.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
                m_PhysicalDevice = device;
                selectedQueueFamilyIndex = graphicsQueueFamily;
            }
        }
    }

    if (!m_PhysicalDevice) {
        LogError("Vulkan: Failed to find suitable physical device");
        return nullptr;
    }

    m_GraphicsQueueFamilyIndex = selectedQueueFamilyIndex;

    float queuePriority = 1.0f;
    vk::DeviceQueueCreateInfo queueCreateInfo;
    queueCreateInfo.queueFamilyIndex = static_cast<uint32_t>(selectedQueueFamilyIndex);
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
#ifdef __APPLE__
    // MoltenVK 属于 portability 实现，若设备报告 VK_KHR_portability_subset 则必须启用
    {
        auto availableExtensions = m_PhysicalDevice.enumerateDeviceExtensionProperties();
        for (const auto& ext : availableExtensions) {
            if (strcmp(ext.extensionName, VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME) == 0) {
                deviceExtensions.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
                break;
            }
        }
    }
#endif

    vk::PhysicalDeviceVulkan12Features vulkan12Features;
    vulkan12Features.timelineSemaphore = VK_TRUE;

    vk::PhysicalDeviceVulkan13Features vulkan13Features;
    vulkan13Features.dynamicRendering = VK_TRUE;
    vulkan13Features.synchronization2 = VK_TRUE;
    vulkan13Features.pNext = &vulkan12Features;

    vk::DeviceCreateInfo deviceCreateInfo;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
    deviceCreateInfo.setPEnabledExtensionNames(deviceExtensions);
    deviceCreateInfo.pNext = &vulkan13Features;

    try {
        m_Device = m_PhysicalDevice.createDevice(deviceCreateInfo);
    } catch (const vk::SystemError& e) {
        LogError("Vulkan: Failed to create device: " + std::string(e.what()));
        return nullptr;
    }

    m_GraphicsQueue = m_Device.getQueue(static_cast<uint32_t>(selectedQueueFamilyIndex), 0);

    try {
        m_ImageAvailableSemaphore = m_Device.createSemaphore({});
        m_RenderFinishedSemaphore = m_Device.createSemaphore({});
        m_InFlightFence = m_Device.createFence({ vk::FenceCreateFlagBits::eSignaled });
    } catch (const vk::SystemError& e) {
        LogError("Vulkan: Failed to create synchronization objects: " + std::string(e.what()));
        return nullptr;
    }

    nvrhi::vulkan::DeviceDesc nvrhiDesc;
    nvrhiDesc.errorCB = messageCallback;
    nvrhiDesc.instance = m_Instance;
    nvrhiDesc.physicalDevice = m_PhysicalDevice;
    nvrhiDesc.device = m_Device;
    nvrhiDesc.graphicsQueue = m_GraphicsQueue;
    nvrhiDesc.graphicsQueueIndex = selectedQueueFamilyIndex;

    m_NvrhiDevice = nvrhi::vulkan::createDevice(nvrhiDesc);
    if (!m_NvrhiDevice) {
        LogError("Vulkan: Failed to create NVRHI device");
        return nullptr;
    }

    if (enableValidation) {
        return nvrhi::validation::createValidationLayer(m_NvrhiDevice);
    }

    return m_NvrhiDevice;
}

void VulkanContext::createSwapChain(void* windowHandle, int width, int height, int backBufferCount) {
    if (!m_Instance || !m_Device) {
        LogError("Vulkan: Cannot create swap chain before device");
        return;
    }

#ifdef _WIN32
    HWND hwnd = static_cast<HWND>(windowHandle);

    vk::Win32SurfaceCreateInfoKHR surfaceCreateInfo;
    surfaceCreateInfo.hinstance = GetModuleHandle(nullptr);
    surfaceCreateInfo.hwnd = hwnd;

    try {
        m_Surface = m_Instance.createWin32SurfaceKHR(surfaceCreateInfo);
    } catch (const vk::SystemError& e) {
        LogError("Vulkan: Failed to create Win32 surface: " + std::string(e.what()));
        return;
    }
#elif defined(__APPLE__)
    // windowHandle 为 NSView*：挂接 CAMetalLayer 后创建 Metal surface
    void* caMetalLayer = CreateMetalLayerForView(windowHandle);
    if (!caMetalLayer) {
        LogError("Vulkan: Failed to attach CAMetalLayer to NSView");
        return;
    }

    vk::MetalSurfaceCreateInfoEXT surfaceCreateInfo;
    surfaceCreateInfo.pLayer = static_cast<const CAMetalLayer*>(caMetalLayer);

    try {
        m_Surface = m_Instance.createMetalSurfaceEXT(surfaceCreateInfo);
    } catch (const vk::SystemError& e) {
        LogError("Vulkan: Failed to create Metal surface: " + std::string(e.what()));
        return;
    }
#else
    LogError("Vulkan: Unsupported platform for surface creation");
    return;
#endif

    vk::Bool32 presentSupport = m_PhysicalDevice.getSurfaceSupportKHR(
        static_cast<uint32_t>(m_GraphicsQueueFamilyIndex), m_Surface);
    if (!presentSupport) {
        LogError("Vulkan: Graphics queue does not support presentation");
        return;
    }

    auto capabilities = m_PhysicalDevice.getSurfaceCapabilitiesKHR(m_Surface);

    auto formats = m_PhysicalDevice.getSurfaceFormatsKHR(m_Surface);

    vk::Format desiredVkFormat = static_cast<vk::Format>(nvrhi::vulkan::convertFormat(m_SwapChainFormat));
    vk::SurfaceFormatKHR selectedFormat;
    bool formatFound = false;
    for (const auto& format : formats) {
        if (format.format == desiredVkFormat &&
            format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            selectedFormat = format;
            formatFound = true;
            break;
        }
    }
    if (!formatFound) {
        LogError("Vulkan: Desired swap chain format is not supported by the surface");
        return;
    }

    auto presentModes = m_PhysicalDevice.getSurfacePresentModesKHR(m_Surface);

    vk::PresentModeKHR selectedPresentMode = vk::PresentModeKHR::eFifo;
    for (const auto& mode : presentModes) {
        if (mode == vk::PresentModeKHR::eMailbox) {
            selectedPresentMode = mode;
            break;
        }
    }

    vk::Extent2D extent;
    if (capabilities.currentExtent.width != UINT32_MAX) {
        extent = capabilities.currentExtent;
    } else {
        extent.width = static_cast<uint32_t>(width);
        extent.height = static_cast<uint32_t>(height);
        extent.width = std::max(capabilities.minImageExtent.width,
                                std::min(capabilities.maxImageExtent.width, extent.width));
        extent.height = std::max(capabilities.minImageExtent.height,
                                 std::min(capabilities.maxImageExtent.height, extent.height));
    }

    uint32_t imageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
        imageCount = capabilities.maxImageCount;
    }

    vk::SwapchainCreateInfoKHR swapChainCreateInfo;
    swapChainCreateInfo.surface = m_Surface;
    swapChainCreateInfo.minImageCount = imageCount;
    swapChainCreateInfo.imageFormat = selectedFormat.format;
    swapChainCreateInfo.imageColorSpace = selectedFormat.colorSpace;
    swapChainCreateInfo.imageExtent = extent;
    swapChainCreateInfo.imageArrayLayers = 1;
    swapChainCreateInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst;
    swapChainCreateInfo.imageSharingMode = vk::SharingMode::eExclusive;
    swapChainCreateInfo.preTransform = capabilities.currentTransform;
    swapChainCreateInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    swapChainCreateInfo.presentMode = selectedPresentMode;
    swapChainCreateInfo.clipped = VK_TRUE;
    swapChainCreateInfo.oldSwapchain = nullptr;

    try {
        m_SwapChain = m_Device.createSwapchainKHR(swapChainCreateInfo);
    } catch (const vk::SystemError& e) {
        LogError("Vulkan: Failed to create swap chain: " + std::string(e.what()));
        return;
    }

    auto swapChainImages = m_Device.getSwapchainImagesKHR(m_SwapChain);

    // 注意：必须用交换链的实际 extent（像素），而非窗口逻辑尺寸
    // （macOS Retina 下二者相差 backingScaleFactor 倍）
    m_Width = static_cast<int>(extent.width);
    m_Height = static_cast<int>(extent.height);
    m_BackBufferCount = static_cast<int>(swapChainImages.size());

    m_SwapChainBuffers.resize(swapChainImages.size());
    for (size_t i = 0; i < swapChainImages.size(); ++i) {
        nvrhi::TextureDesc texDesc;
        texDesc.width = extent.width;
        texDesc.height = extent.height;
        texDesc.format = m_SwapChainFormat;
        texDesc.isRenderTarget = true;
        texDesc.initialState = nvrhi::ResourceStates::Present;
        texDesc.keepInitialState = true;
        texDesc.debugName = "BackBuffer_" + std::to_string(i);

        m_SwapChainBuffers[i] = m_NvrhiDevice->createHandleForNativeTexture(
            nvrhi::ObjectTypes::VK_Image,
            static_cast<VkImage>(swapChainImages[i]),
            texDesc
        );
    }
}

nvrhi::ITexture* VulkanContext::getBackBuffer(size_t index) {
    if (index >= m_SwapChainBuffers.size()) {
        return nullptr;
    }
    return m_SwapChainBuffers[index];
}

size_t VulkanContext::getCurrentBackBufferIndex() {
    if (!m_SwapChain) {
        return 0;
    }

    auto result = m_Device.waitForFences(m_InFlightFence, VK_TRUE, UINT64_MAX);
    if (result != vk::Result::eSuccess) {
        LogError("Vulkan: Failed to wait for fence");
        return 0;
    }

    m_Device.resetFences(m_InFlightFence);

    auto acquireResult = m_Device.acquireNextImageKHR(
        m_SwapChain,
        UINT64_MAX,
        m_ImageAvailableSemaphore,
        nullptr
    );

    if (acquireResult.result != vk::Result::eSuccess &&
        acquireResult.result != vk::Result::eSuboptimalKHR) {
        LogError("Vulkan: Failed to acquire swap chain image");
        return 0;
    }

    // 提交一个空 submit 等待 acquire semaphore：渲染命令由 NVRHI 内部提交、
    // 无法注入 semaphore，因此用这个空 submit 建立依赖，保证后续渲染在演示
    // 引擎释放图像之后才开始执行（否则触发 validation 错误：
    // non-acquired-swapchain-image-used）。
    vk::PipelineStageFlags acquireWaitStages[] = {
        vk::PipelineStageFlagBits::eColorAttachmentOutput
    };
    vk::SubmitInfo acquireSubmitInfo;
    acquireSubmitInfo.waitSemaphoreCount = 1;
    acquireSubmitInfo.pWaitSemaphores = &m_ImageAvailableSemaphore;
    acquireSubmitInfo.pWaitDstStageMask = acquireWaitStages;

    try {
        m_GraphicsQueue.submit(acquireSubmitInfo);
    } catch (const vk::SystemError& e) {
        LogError("Vulkan: Failed to submit acquire wait: " + std::string(e.what()));
        return 0;
    }

    m_CurrentImageIndex = acquireResult.value;
    return static_cast<size_t>(m_CurrentImageIndex);
}

void VulkanContext::present(bool vsync) {
    if (!m_SwapChain) {
        return;
    }

    // 空 submit：渲染命令由 NVRHI 内部提交（先于本 submit 入队，同队列按提交顺序执行），
    // 这里只负责发出 RenderFinished 信号并携带 fence 供下一帧等待
    vk::SubmitInfo submitInfo;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &m_RenderFinishedSemaphore;

    try {
        m_GraphicsQueue.submit(submitInfo, m_InFlightFence);
    } catch (const vk::SystemError& e) {
        LogError("Vulkan: Failed to submit draw command buffer: " + std::string(e.what()));
        return;
    }

    vk::PresentInfoKHR presentInfo;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &m_RenderFinishedSemaphore;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &m_SwapChain;
    presentInfo.pImageIndices = &m_CurrentImageIndex;

    try {
        auto result = m_GraphicsQueue.presentKHR(presentInfo);
        if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
            LogError("Vulkan: Failed to present swap chain image");
        }
    } catch (const vk::SystemError& e) {
        LogError("Vulkan: Failed to present swap chain image: " + std::string(e.what()));
    }
}

void VulkanContext::waitForIdle() {
    if (m_Device) {
        m_Device.waitIdle();
    }
}

} // namespace nvrhi_lab
