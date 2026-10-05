#include "basic_renderer.h"
#include "device_manager.h"
#include "../utils/logging.h"

#include <nvrhi/utils.h>
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>

#ifdef _WIN32
    #include <windows.h>
#elif defined(__APPLE__)
    #include <mach-o/dyld.h>
#else
    #include <unistd.h>
#endif

namespace nvrhi_lab {

namespace {

struct Vertex {
    float position[3];
    float color[4];
};

static const Vertex g_Vertices[] = {
    { {  0.0f,  0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
    { {  0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
    { { -0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
};

// 可执行文件所在目录（bin/）。shader 查找基于它而非 CWD，
// 使任何启动方式（VSCode launch / terminal / 双击）都能定位 bin/shaders/
std::filesystem::path ExecutableDir()
{
#ifdef _WIN32
    wchar_t path[MAX_PATH] = {};
    GetModuleFileNameW(nullptr, path, MAX_PATH);
    return std::filesystem::path(path).parent_path();
#elif defined(__APPLE__)
    uint32_t size = 0;
    _NSGetExecutablePath(nullptr, &size);
    std::vector<char> buffer(size);
    _NSGetExecutablePath(buffer.data(), &size);
    return std::filesystem::path(buffer.data()).lexically_normal().parent_path();
#else
    return std::filesystem::read_symlink("/proc/self/exe").lexically_normal().parent_path();
#endif
}

std::string GetShaderSubdirectory(GraphicsBackend backend) {
    const char* subDir = "";
    switch (backend) {
        case GraphicsBackend::D3D11:  subDir = "d3d11";  break;
        case GraphicsBackend::D3D12:  subDir = "d3d12";  break;
        case GraphicsBackend::Vulkan: subDir = "vulkan"; break;
        default: break;
    }
    return (ExecutableDir() / "shaders" / subDir).string() + "/";
}

std::string GetShaderExtension(GraphicsBackend backend) {
    switch (backend) {
        case GraphicsBackend::D3D11:
        case GraphicsBackend::D3D12:
            return ".cso";
        case GraphicsBackend::Vulkan:
            return ".spv";
        default:
            return ".cso";
    }
}

std::vector<uint8_t> LoadShaderFile(const std::string& filename, GraphicsBackend backend) {
    std::string shaderPath = GetShaderSubdirectory(backend) + filename;
    std::ifstream file(shaderPath, std::ios::binary | std::ios::ate);
    
    if (!file.is_open()) {
        LogError("Failed to open shader file: " + shaderPath);
        return {};
    }
    
    size_t fileSize = static_cast<size_t>(file.tellg());
    file.seekg(0);
    
    std::vector<uint8_t> buffer(fileSize);
    file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
    file.close();
    
    return buffer;
}

} // anonymous namespace

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

    // Acceptance check (tier 3): project a known point through the VP matrix and
    // log the NDC result. Triangle top vertex (0, 0.5, 0) from (0, 0, -3), fov 45:
    // expect x ~= 0, |y| ~= 0.40, z ~= 0.97. y is +0.40 on D3D (+Y up = top) and
    // -0.40 on Vulkan (flipped here, -Y = top) — i.e. the vertex lands at the TOP
    // of the screen on both backends.
    {
        glm::vec4 clip = m_Camera.GetViewProjectionMatrix() * glm::vec4(0.0f, 0.5f, 0.0f, 1.0f);
        glm::vec3 ndc = clip.w != 0.0f ? glm::vec3(clip / clip.w) : glm::vec3(clip);
        LogInfo("Camera sanity check: top vertex projects to NDC ("
            + std::to_string(ndc.x) + ", " + std::to_string(ndc.y) + ", " + std::to_string(ndc.z) + ")");
    }
    
    if (!m_CameraCB.Create(device, "CameraConstantBuffer")) {
        LogError("BasicRenderer: Failed to create camera constant buffer");
        return false;
    }
    
    m_BindingLayout = device->createBindingLayout(
        nvrhi::BindingLayoutDesc()
            .setVisibility(nvrhi::ShaderType::Vertex)
            .setBindingOffsets(nvrhi::VulkanBindingOffsets()
                .setConstantBufferOffset(0)
                .setShaderResourceOffset(0)
                .setSamplerOffset(0)
                .setUnorderedAccessViewOffset(0))
            .addItem(nvrhi::BindingLayoutItem::ConstantBuffer(0)));
    if (!m_BindingLayout) {
        LogError("BasicRenderer: Failed to create binding layout");
        return false;
    }
    
    m_BindingSet = device->createBindingSet(
        nvrhi::BindingSetDesc()
            .addItem(nvrhi::BindingSetItem::ConstantBuffer(0, m_CameraCB.GetBuffer())),
        m_BindingLayout);
    if (!m_BindingSet) {
        LogError("BasicRenderer: Failed to create binding set");
        return false;
    }
    
    std::string shaderExt = GetShaderExtension(backend);
    
    auto vsBytecode = LoadShaderFile("triangle.vs" + shaderExt, backend);
    if (vsBytecode.empty()) {
        LogError("BasicRenderer: Failed to load vertex shader");
        return false;
    }
    
    auto psBytecode = LoadShaderFile("triangle.ps" + shaderExt, backend);
    if (psBytecode.empty()) {
        LogError("BasicRenderer: Failed to load pixel shader");
        return false;
    }
    
    m_VertexShader = device->createShader(
        nvrhi::ShaderDesc().setShaderType(nvrhi::ShaderType::Vertex).setDebugName("TriangleVS"),
        vsBytecode.data(), vsBytecode.size());
    if (!m_VertexShader) {
        LogError("Failed to create vertex shader");
        return false;
    }
    
    m_PixelShader = device->createShader(
        nvrhi::ShaderDesc().setShaderType(nvrhi::ShaderType::Pixel).setDebugName("TrianglePS"),
        psBytecode.data(), psBytecode.size());
    if (!m_PixelShader) {
        LogError("Failed to create pixel shader");
        return false;
    }
    
    nvrhi::VertexAttributeDesc attributes[] = {
        nvrhi::VertexAttributeDesc()
            .setName("POSITION")
            .setFormat(nvrhi::Format::RGB32_FLOAT)
            .setOffset(offsetof(Vertex, position))
            .setElementStride(sizeof(Vertex)),
        nvrhi::VertexAttributeDesc()
            .setName("COLOR")
            .setFormat(nvrhi::Format::RGBA32_FLOAT)
            .setOffset(offsetof(Vertex, color))
            .setElementStride(sizeof(Vertex))
    };
    
    nvrhi::InputLayoutHandle inputLayout = device->createInputLayout(
        attributes, uint32_t(std::size(attributes)), m_VertexShader);
    
    if (!inputLayout) {
        LogError("Failed to create input layout");
        return false;
    }
    
    m_VertexBuffer = device->createBuffer(
        nvrhi::BufferDesc()
            .setByteSize(sizeof(g_Vertices))
            .setIsVertexBuffer(true)
            .enableAutomaticStateTracking(nvrhi::ResourceStates::VertexBuffer)
            .setDebugName("TriangleVertexBuffer"));
    
    if (!m_VertexBuffer) {
        LogError("Failed to create vertex buffer");
        return false;
    }

    m_vertexBufferBinding.setBuffer(m_VertexBuffer).setSlot(0).setOffset(0);

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
        LogError("Failed to create depth buffer");
        return false;
    }
    
    nvrhi::FramebufferInfo fbInfo;
    fbInfo.addColorFormat(m_DeviceManager->GetSwapChainFormat())
          .setDepthFormat(nvrhi::Format::D32);

    nvrhi::RenderState renderState;
    renderState.depthStencilState.enableDepthTest();
    // NVRHI defaults to back-face culling (RasterCullMode::Back), which hides the
    // triangle for the entire back half of the camera orbit. Disable it so the
    // triangle is visible from both sides — it should only vanish for an instant
    // when edge-on. Re-enable culling deliberately in Phase 2.6 (cube/winding).
    renderState.rasterState.setCullNone();
    
    nvrhi::GraphicsPipelineDesc pipelineDesc;
    pipelineDesc.setInputLayout(inputLayout)
                .setVertexShader(m_VertexShader)
                .setRenderState(renderState)
                .setPixelShader(m_PixelShader)
                .addBindingLayout(m_BindingLayout);
    
    m_Pipeline = device->createGraphicsPipeline(pipelineDesc, fbInfo);
    
    if (!m_Pipeline) {
        LogError("Failed to create graphics pipeline");
        return false;
    }
    
    m_CommandList = device->createCommandList();
    if (!m_CommandList) {
        LogError("Failed to create command list");
        return false;
    }
    
    m_CommandList->open();
    m_CommandList->writeBuffer(m_VertexBuffer, g_Vertices, sizeof(g_Vertices));
    m_CommandList->close();
    device->executeCommandList(m_CommandList);
    
    m_Initialized = true;
    return true;
}

void BasicRenderer::Shutdown() {
    m_BindingSet = nullptr;
    m_BindingLayout = nullptr;
    m_CommandList = nullptr;
    m_Pipeline = nullptr;
    m_VertexBuffer = nullptr;
    m_DepthBuffer = nullptr;
    m_PixelShader = nullptr;
    m_VertexShader = nullptr;
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
    
    nvrhi::GraphicsState state;
    state.setPipeline(m_Pipeline)
         .setFramebuffer(framebuffer)
         .setViewport(nvrhi::ViewportState().addViewportAndScissorRect(
             nvrhi::Viewport(float(m_DeviceManager->GetWidth()), float(m_DeviceManager->GetHeight()))))
         .addVertexBuffer(m_vertexBufferBinding)
         .addBindingSet(m_BindingSet);
    
    m_CommandList->setGraphicsState(state);
    
    nvrhi::DrawArguments args;
    args.setVertexCount(uint32_t(std::size(g_Vertices)));
    m_CommandList->draw(args);
    
    m_CommandList->setTextureState(
        backBuffer,
        nvrhi::AllSubresources,
        nvrhi::ResourceStates::Present);
    m_CommandList->commitBarriers();
    
    m_CommandList->close();
    device->executeCommandList(m_CommandList);
}

} // namespace nvrhi_lab
