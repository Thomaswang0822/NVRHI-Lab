#pragma once

#include <nvrhi/nvrhi.h>
#include "camera.h"
#include "constant_buffer.h"
#include "camera_constants.h"

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

    Camera m_Camera;
    ConstantBuffer<CameraConstants> m_CameraCB;

    nvrhi::ShaderHandle m_VertexShader;
    nvrhi::ShaderHandle m_PixelShader;
    nvrhi::GraphicsPipelineHandle m_Pipeline;
    nvrhi::BindingLayoutHandle m_BindingLayout;
    nvrhi::BindingSetHandle m_BindingSet;
    nvrhi::BufferHandle m_VertexBuffer;
    nvrhi::VertexBufferBinding m_vertexBufferBinding;
    nvrhi::TextureHandle m_DepthBuffer;
    nvrhi::CommandListHandle m_CommandList;
};

} // namespace nvrhi_lab
