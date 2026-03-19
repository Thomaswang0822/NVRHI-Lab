#pragma once

#include <nvrhi/nvrhi.h>
#include <memory>

namespace nvrhi_lab {

class DeviceManager;

// Renders a simple colored triangle using NVRHI.
// Owns pipeline state, shaders, vertex buffer, and command list.
// Depends on DeviceManager for device and backbuffer access.
class BasicRenderer {
public:
    BasicRenderer();
    ~BasicRenderer();

    // Initialize renderer with a DeviceManager. Creates all GPU resources.
    // Returns false if any resource creation fails.
    bool Initialize(DeviceManager* deviceManager);

    // Release all GPU resources.
    void Shutdown();

    // Record and submit draw commands for the current frame.
    // Should be called after DeviceManager::BeginFrame().
    void Render();

    bool IsInitialized() const { return m_Initialized; }

private:
    bool CreateShaders();
    bool CreatePipeline();
    bool CreateVertexBuffer();
    bool CreateCommandList();

    DeviceManager* m_DeviceManager = nullptr;
    bool m_Initialized = false;

    nvrhi::ShaderHandle m_VertexShader;
    nvrhi::ShaderHandle m_PixelShader;
    nvrhi::GraphicsPipelineHandle m_Pipeline;
    nvrhi::BufferHandle m_VertexBuffer;
    nvrhi::CommandListHandle m_CommandList;
};

} // namespace nvrhi_lab
