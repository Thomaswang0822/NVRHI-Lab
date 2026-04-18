#include "basic_renderer.h"
#include "device_manager.h"
#include "../utils/logging.h"

#include <nvrhi/utils.h>
#include <fstream>
#include <vector>

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

std::string GetShaderSubdirectory(GraphicsBackend backend) {
    switch (backend) {
        case GraphicsBackend::D3D11: return "shaders/d3d11/";
        case GraphicsBackend::D3D12: return "shaders/d3d12/";
        case GraphicsBackend::Vulkan: return "shaders/vulkan/";
        default: return "shaders/";
    }
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

BasicRenderer::BasicRenderer() = default;

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
    m_Camera.LookAt(glm::vec3(0.0f, 0.0f, -3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    m_Camera.SetPerspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
    
    if (!m_CameraCB.Create(device, "CameraConstantBuffer")) {
        LogError("BasicRenderer: Failed to create camera constant buffer");
        return false;
    }
    
    m_BindingLayout = device->createBindingLayout(
        nvrhi::BindingLayoutDesc()
            .setVisibility(nvrhi::ShaderType::Vertex)
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
