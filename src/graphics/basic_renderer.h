#pragma once

#include <nvrhi/nvrhi.h>

namespace nvrhi_lab {

class DeviceManager;

class BasicRenderer {
public:
    BasicRenderer();
    ~BasicRenderer();

    bool Initialize(DeviceManager* deviceManager);
    void Shutdown();
    void Render();

    bool IsInitialized() const { return m_Initialized; }

private:
    DeviceManager* m_DeviceManager = nullptr;
    bool m_Initialized = false;

    nvrhi::ShaderHandle m_VertexShader;
    nvrhi::ShaderHandle m_PixelShader;
    nvrhi::GraphicsPipelineHandle m_Pipeline;
    nvrhi::BufferHandle m_VertexBuffer;
	nvrhi::VertexBufferBinding m_vertexBufferBinding;
    nvrhi::CommandListHandle m_CommandList;
};

} // namespace nvrhi_lab
