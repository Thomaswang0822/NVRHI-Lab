#pragma once

#include <cstdint>
#include "camera.h"

namespace nvrhi_lab {

// 语义化的相机输入位。前端（wxWidgets）把原始按键映射到这些位，
// 图形层不依赖任何 UI 框架
enum CameraInput : uint32_t {
    InputNone  = 0,
    MoveForward = 1u << 0,
    MoveBack    = 1u << 1,
    MoveLeft    = 1u << 2,
    MoveRight   = 1u << 3,
    MoveUp      = 1u << 4,
    MoveDown    = 1u << 5,
    LookLeft    = 1u << 6,
    LookRight   = 1u << 7,
    LookUp      = 1u << 8,
    LookDown    = 1u << 9,
    TurnCCW     = 1u << 10,  // 原地左转（俯视逆时针，绕相机自身竖直轴）
    TurnCW      = 1u << 11,  // 原地右转（俯视顺时针）
};

// 上帝模式自由相机：yaw/pitch 姿态 + 六向平移，一切以相机自身为中心，
// 与场景中任何物体零耦合（物体可能被移/转出视野）。每个 Update 用当前
// 姿态驱动 Camera（view matrix）。这是相机系统的 tier-1 acceptance
// check：交互控制下任何矩阵/姿态错误都能立即观察到
class CameraController {
public:
    explicit CameraController(Camera& camera);

    void Reset();                           // 回到初始视角 (0, 0, -3) → 原点
    void Update(float dt, uint32_t input);  // 应用输入，刷新 Camera

    void SetMoveSpeed(float unitsPerSecond) { m_MoveSpeed = unitsPerSecond; }
    void SetRotateSpeed(float radiansPerSecond) { m_RotateSpeed = radiansPerSecond; }

    const glm::vec3& GetPosition() const { return m_Position; }
    float GetYaw() const { return m_Yaw; }
    float GetPitch() const { return m_Pitch; }

private:
    void ComputeBasis(glm::vec3& forward, glm::vec3& right, glm::vec3& up) const;
    void ApplyToCamera();

    Camera& m_Camera;

    glm::vec3 m_Position{0.0f, 0.0f, -3.0f};
    float m_Yaw = 0.0f;   // + = 向右看（XZ 平面，左手系）
    float m_Pitch = 0.0f; // + = 向上看，钳制在 ±89°

    float m_MoveSpeed = 3.0f;    // units/s
    float m_RotateSpeed = 1.5f;  // rad/s
};

} // namespace nvrhi_lab
