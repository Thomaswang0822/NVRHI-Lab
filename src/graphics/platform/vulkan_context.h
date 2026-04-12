#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>
#include <nvrhi/vulkan.h>

#include <vector>

#include "platform_context.h"

namespace nvrhi_lab {

class VulkanContext : public IPlatformContext {
public:
    VulkanContext();
    ~VulkanContext();

    nvrhi::DeviceHandle createNvrhiDevice(nvrhi::IMessageCallback* messageCallback, bool enableValidation) override;

    void createSwapChain(void* windowHandle, int width, int height, int backBufferCount) override;
    nvrhi::ITexture* getBackBuffer(size_t index) override;
    size_t getCurrentBackBufferIndex() override;
    void present(bool vsync) override;

    void waitForIdle() override;

    const char* getBackendName() const override { return "Vulkan"; }

private:
    int m_BackBufferCount = 0;

    vk::Instance m_Instance;
    vk::PhysicalDevice m_PhysicalDevice;
    vk::Device m_Device;
    vk::Queue m_GraphicsQueue;
    int m_GraphicsQueueFamilyIndex = 0;
    vk::SurfaceKHR m_Surface;
    vk::SwapchainKHR m_SwapChain;
    vk::Semaphore m_ImageAvailableSemaphore;
    vk::Semaphore m_RenderFinishedSemaphore;
    vk::Fence m_InFlightFence;
    uint32_t m_CurrentImageIndex = 0;

    nvrhi::DeviceHandle m_NvrhiDevice;
    std::vector<nvrhi::TextureHandle> m_SwapChainBuffers;
};

} // namespace nvrhi_lab
