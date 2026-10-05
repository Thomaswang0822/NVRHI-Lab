#include "basic_renderer.h"
#include "device_manager.h"
#include "../utils/logging.h"

#include <nvrhi/utils.h>

namespace nvrhi_lab {

BasicRenderer::BasicRenderer()
    : m_CameraController(m_Camera)
{
}

BasicRenderer::~BasicRenderer() {
    Shutdown();
}

bool BasicRenderer::Initialize(DeviceManager* deviceManager) {
    if (m_Initialized) {
        return true;
    }

    if (!deviceManager || !deviceManager->IsInitialized()) {
        LogError("BasicRenderer: DeviceManager is null or not initialized");
        return false;
    }

    m_DeviceManager = deviceManager;

    nvrhi::IDevice* device = m_DeviceManager->GetDevice();
    GraphicsBackend backend = m_DeviceManager->GetBackend();

    float aspect = float(m_DeviceManager->GetWidth()) / float(m_DeviceManager->GetHeight());
    // 初始视角由 CameraController::Reset() 设置（构造时），即 (0, 0, -3) → 原点
    m_Camera.SetPerspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
    // Vulkan NDC +Y points down the screen; flip the projection so both backends
    // render the same image (Phase 1 left the triangle vertically mirrored on Vulkan)
    m_Camera.SetFlipY(backend == GraphicsBackend::Vulkan);
    m_LastFrameTime = std::chrono::steady_clock::now();

    // Acceptance check (tier 3): project a known world point through the VP
    // matrix and log the NDC result. Cube top-face center (0, 0.5, 0) from
    // (0, 0, -3), fov 45: expect x ~= 0, |y| ~= 0.40, z ~= 0.97. y is +0.40 on
    // D3D (+Y up = top) and -0.40 on Vulkan (flipped here, -Y = top) — i.e.
    // the point lands at the TOP of the screen on both backends.
    {
        glm::vec4 clip = m_Camera.GetViewProjectionMatrix() * glm::vec4(0.0f, 0.5f, 0.0f, 1.0f);
        glm::vec3 ndc = clip.w != 0.0f ? glm::vec3(clip / clip.w) : glm::vec3(clip);
        LogInfo("Camera sanity check: (0, 0.5, 0) projects to NDC ("
            + std::to_string(ndc.x) + ", " + std::to_string(ndc.y) + ", " + std::to_string(ndc.z) + ")");
    }

    if (!m_CameraCB.Create(device, "CameraConstantBuffer")) {
        LogError("BasicRenderer: Failed to create camera constant buffer");
        return false;
    }

    auto depthDesc = nvrhi::TextureDesc()
        .setDimension(nvrhi::TextureDimension::Texture2D)
        .setWidth(m_DeviceManager->GetWidth())
        .setHeight(m_DeviceManager->GetHeight())
        .setFormat(nvrhi::Format::D32)
        .setIsRenderTarget(true)
        .setClearValue(1.0f)
        .setUseClearValue(true)
        .setDebugName("DepthBuffer")
        .enableAutomaticStateTracking(nvrhi::ResourceStates::DepthWrite);
    depthDesc.isShaderResource = false;  // Depth buffer is for depth testing only, not sampled in shaders

    m_DepthBuffer = device->createTexture(depthDesc);
    if (!m_DepthBuffer) {
        LogError("BasicRenderer: Failed to create depth buffer");
        return false;
    }

    m_CommandList = device->createCommandList();
    if (!m_CommandList) {
        LogError("BasicRenderer: Failed to create command list");
        return false;
    }

    // 场景：背景墙 + 靶子立方体（替代已删除的测试三角形）
    m_Scene = std::make_unique<SceneManager>();
    if (!m_Scene->Create(device, m_CameraCB.GetBuffer(), backend,
                         m_DeviceManager->GetSwapChainFormat())) {
        LogError("BasicRenderer: Failed to create scene");
        return false;
    }

    m_Initialized = true;
    return true;
}

void BasicRenderer::Shutdown() {
    if (m_Scene) {
        m_Scene->Shutdown();
        m_Scene.reset();
    }
    m_CommandList = nullptr;
    m_DepthBuffer = nullptr;
    m_DeviceManager = nullptr;
    m_Initialized = false;
}

void BasicRenderer::Render() {
    if (!m_Initialized || !m_DeviceManager) {
        return;
    }

    nvrhi::IDevice* device = m_DeviceManager->GetDevice();
    nvrhi::ITexture* backBuffer = m_DeviceManager->GetCurrentBackBuffer();

    if (!backBuffer) {
        return;
    }

    // Tier-1 acceptance check：上帝模式交互相机（WASD/R/F 平移 + 方向键
    // look + Q/E 原地转向，前端喂 CameraInput 位掩码）。姿态/矩阵/翻转
    // 任何一环出错，动一下按键就能看到
    auto now = std::chrono::steady_clock::now();
    float dt = std::chrono::duration<float>(now - m_LastFrameTime).count();
    m_LastFrameTime = now;
    m_CameraController.Update(dt, m_CameraInputMask);

    nvrhi::FramebufferDesc fbDesc;
    fbDesc.addColorAttachment(backBuffer)
          .setDepthAttachment(m_DepthBuffer);

    nvrhi::FramebufferHandle framebuffer = device->createFramebuffer(fbDesc);

    m_CommandList->open();

    m_CameraCB.Update(m_CommandList, m_Camera.GetConstants());

    nvrhi::utils::ClearColorAttachment(m_CommandList, framebuffer, 0, nvrhi::Color(0.0f, 0.0f, 0.0f, 1.0f));
    nvrhi::utils::ClearDepthStencilAttachment(m_CommandList, framebuffer, 1.0f, 0);

    // 共享的基础 state（framebuffer/viewport）——各场景对象在此基础上
    // 填充自己的 pipeline/几何缓冲/binding set
    nvrhi::GraphicsState baseState;
    baseState.setFramebuffer(framebuffer)
             .setViewport(nvrhi::ViewportState().addViewportAndScissorRect(
                 nvrhi::Viewport(float(m_DeviceManager->GetWidth()), float(m_DeviceManager->GetHeight()))));

    m_Scene->Render(m_CommandList, baseState);

    m_CommandList->setTextureState(
        backBuffer,
        nvrhi::AllSubresources,
        nvrhi::ResourceStates::Present);
    m_CommandList->commitBarriers();

    m_CommandList->close();
    device->executeCommandList(m_CommandList);
}

} // namespace nvrhi_lab
