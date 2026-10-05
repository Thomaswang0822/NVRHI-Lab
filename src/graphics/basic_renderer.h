#pragma once

#include <nvrhi/nvrhi.h>
#include <chrono>
#include <cstdint>
#include "camera.h"
#include "camera_controller.h"
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

    // 前端每帧把语义化按键状态（CameraInput 位掩码）喂进来，由
    // CameraController 在 Render 时消费
    void SetCameraInput(uint32_t inputMask) { m_CameraInputMask = inputMask; }

    bool IsInitialized() const { return m_Initialized; }

private:
    DeviceManager* m_DeviceManager = nullptr;
    bool m_Initialized = false;

    Camera m_Camera;
    CameraController m_CameraController;
    ConstantBuffer<CameraConstants> m_CameraCB;
    std::chrono::steady_clock::time_point m_LastFrameTime;
    uint32_t m_CameraInputMask = InputNone;

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
