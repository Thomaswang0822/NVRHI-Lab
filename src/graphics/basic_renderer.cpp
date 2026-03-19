#include "basic_renderer.h"
#include "device_manager.h"

#include <fstream>
#include <vector>
#include <iostream>

namespace nvrhi_lab {

namespace {

struct Vertex {
    float position[3];
    float color[4];
};

std::vector<uint8_t> LoadShaderFile(const std::string& filename) {
    std::string shaderPath = "shaders/" + filename;
    std::ifstream file(shaderPath, std::ios::binary | std::ios::ate);
    
    if (!file.is_open()) {
        std::cerr << "Failed to open shader file: " << shaderPath << std::endl;
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
        std::cerr << "BasicRenderer: DeviceManager is null or not initialized" << std::endl;
        return false;
    }
    
    m_DeviceManager = deviceManager;
    
    if (!CreateShaders()) {
        std::cerr << "BasicRenderer: Failed to create shaders" << std::endl;
        Shutdown();
        return false;
    }
    
    if (!CreateVertexBuffer()) {
        std::cerr << "BasicRenderer: Failed to create vertex buffer" << std::endl;
        Shutdown();
        return false;
    }
    
    if (!CreatePipeline()) {
        std::cerr << "BasicRenderer: Failed to create pipeline" << std::endl;
        Shutdown();
        return false;
    }
    
    if (!CreateCommandList()) {
        std::cerr << "BasicRenderer: Failed to create command list" << std::endl;
        Shutdown();
        return false;
    }
    
    m_Initialized = true;
    return true;
}

void BasicRenderer::Shutdown() {
    m_CommandList = nullptr;
    m_Pipeline = nullptr;
    m_VertexBuffer = nullptr;
    m_PixelShader = nullptr;
    m_VertexShader = nullptr;
    m_DeviceManager = nullptr;
    m_Initialized = false;
}

bool BasicRenderer::CreateShaders() {
    nvrhi::IDevice* device = m_DeviceManager->GetDevice();
    
    auto vsBytecode = LoadShaderFile("triangle.vs.cso");
    if (vsBytecode.empty()) {
        return false;
    }
    
    auto psBytecode = LoadShaderFile("triangle.ps.cso");
    if (psBytecode.empty()) {
        return false;
    }
    
    nvrhi::ShaderDesc vsDesc;
    vsDesc.setShaderType(nvrhi::ShaderType::Vertex)
          .setDebugName("TriangleVS");
    
    m_VertexShader = device->createShader(vsDesc, vsBytecode.data(), vsBytecode.size());
    if (!m_VertexShader) {
        std::cerr << "Failed to create vertex shader" << std::endl;
        return false;
    }
    
    nvrhi::ShaderDesc psDesc;
    psDesc.setShaderType(nvrhi::ShaderType::Pixel)
          .setDebugName("TrianglePS");
    
    m_PixelShader = device->createShader(psDesc, psBytecode.data(), psBytecode.size());
    if (!m_PixelShader) {
        std::cerr << "Failed to create pixel shader" << std::endl;
        return false;
    }
    
    return true;
}

bool BasicRenderer::CreatePipeline() {
    nvrhi::IDevice* device = m_DeviceManager->GetDevice();
    
    nvrhi::VertexAttributeDesc attributes[] = {
        nvrhi::VertexAttributeDesc()
            .setName("POSITION")
            .setFormat(nvrhi::Format::RGB32_FLOAT)
            .setOffset(0)
            .setElementStride(sizeof(Vertex))
            .setBufferIndex(0),
        nvrhi::VertexAttributeDesc()
            .setName("COLOR")
            .setFormat(nvrhi::Format::RGBA32_FLOAT)
            .setOffset(12)
            .setElementStride(sizeof(Vertex))
            .setBufferIndex(0)
    };
    
    nvrhi::InputLayoutHandle inputLayout = device->createInputLayout(
        attributes, 
        sizeof(attributes) / sizeof(attributes[0]), 
        m_VertexShader);
    
    if (!inputLayout) {
        std::cerr << "Failed to create input layout" << std::endl;
        return false;
    }
    
    nvrhi::FramebufferInfo fbInfo;
    fbInfo.addColorFormat(nvrhi::Format::RGBA16_FLOAT)
           .setDepthFormat(nvrhi::Format::D32)
           .setSampleCount(1)
           .setSampleQuality(0);
    
    nvrhi::GraphicsPipelineDesc pipelineDesc;
    pipelineDesc.setVertexShader(m_VertexShader)
                .setPixelShader(m_PixelShader)
                .setInputLayout(inputLayout)
                .setPrimType(nvrhi::PrimitiveType::TriangleList);
    
    m_Pipeline = device->createGraphicsPipeline(pipelineDesc, fbInfo);
    
    if (!m_Pipeline) {
        std::cerr << "Failed to create graphics pipeline" << std::endl;
        return false;
    }
    
    return true;
}

bool BasicRenderer::CreateVertexBuffer() {
    nvrhi::IDevice* device = m_DeviceManager->GetDevice();
    
    Vertex vertices[] = {
        { {  0.0f,  0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
        { {  0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
        { { -0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
    };
    
    nvrhi::BufferDesc bufferDesc;
    bufferDesc.setByteSize(sizeof(vertices))
              .setIsVertexBuffer(true)
              .setDebugName("TriangleVertexBuffer")
              .setInitialState(nvrhi::ResourceStates::VertexBuffer)
              .setKeepInitialState(true);
    
    m_VertexBuffer = device->createBuffer(bufferDesc);
    
    if (!m_VertexBuffer) {
        std::cerr << "Failed to create vertex buffer" << std::endl;
        return false;
    }
    
    nvrhi::CommandListHandle uploadCmdList = device->createCommandList();
    uploadCmdList->open();
    uploadCmdList->writeBuffer(m_VertexBuffer, vertices, sizeof(vertices));
    uploadCmdList->close();
    device->executeCommandList(uploadCmdList);
    
    return true;
}

bool BasicRenderer::CreateCommandList() {
    nvrhi::IDevice* device = m_DeviceManager->GetDevice();
    
    m_CommandList = device->createCommandList();
    
    if (!m_CommandList) {
        std::cerr << "Failed to create command list" << std::endl;
        return false;
    }
    
    return true;
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
    
    m_CommandList->open();
    
    m_CommandList->setTextureState(
        backBuffer,
        nvrhi::AllSubresources,
        nvrhi::ResourceStates::RenderTarget);
    m_CommandList->commitBarriers();
    
    m_CommandList->clearTextureFloat(
        backBuffer,
        nvrhi::AllSubresources,
        nvrhi::Color(0.1f, 0.1f, 0.2f, 1.0f));
    
    nvrhi::FramebufferDesc fbDesc;
    fbDesc.addColorAttachment(backBuffer);
    
    nvrhi::FramebufferHandle framebuffer = device->createFramebuffer(fbDesc);
    
    nvrhi::GraphicsState state;
    state.pipeline = m_Pipeline;
    state.framebuffer = framebuffer;
    state.viewport.addViewportAndScissorRect(
        nvrhi::Viewport(m_DeviceManager->GetWidth(), m_DeviceManager->GetHeight()));
    state.vertexBuffers = {
        nvrhi::VertexBufferBinding()
            .setBuffer(m_VertexBuffer)
            .setSlot(0)
            .setOffset(0)
    };
    
    m_CommandList->setGraphicsState(state);
    
    nvrhi::DrawArguments args;
    args.setVertexCount(3);
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
