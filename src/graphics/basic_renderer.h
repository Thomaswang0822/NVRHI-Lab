#pragma once

#include <nvrhi/nvrhi.h>
#include <chrono>
#include <cstdint>
#include <memory>
#include "camera.h"
#include "camera_controller.h"
#include "constant_buffer.h"
#include "camera_constants.h"
#include "scene_manager.h"

namespace nvrhi_lab {

class DeviceManager;

// 帧级编排：相机（+输入）、相机常量缓冲、深度缓冲、framebuffer、
// command list、清屏与 present。场景物体（墙/靶子等）由 SceneManager
// 管理并绘制——renderer 不再持有任何具体几何体
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

    std::unique_ptr<SceneManager> m_Scene;
    nvrhi::TextureHandle m_DepthBuffer;
    nvrhi::CommandListHandle m_CommandList;
};

} // namespace nvrhi_lab
