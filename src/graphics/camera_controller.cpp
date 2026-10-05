#include "camera_controller.h"

#include <glm/glm.hpp>

namespace nvrhi_lab {

CameraController::CameraController(Camera& camera)
    : m_Camera(camera)
{
    Reset();
}

void CameraController::Reset() {
    m_Position = glm::vec3(0.0f, 0.0f, -3.0f);
    m_Yaw = 0.0f;
    m_Pitch = 0.0f;
    ApplyToCamera();
}

void CameraController::Update(float dt, uint32_t input) {
    // 姿态：yaw 绕相机自身竖直轴（原地转身，地平线保持水平），
    // pitch 绕相机右轴。Q/E 与左右方向键同为原地转向——上帝模式，
    // 与场景物体零耦合，物体可能被转出视野之外
    if (input & LookRight) m_Yaw += m_RotateSpeed * dt;
    if (input & LookLeft)  m_Yaw -= m_RotateSpeed * dt;
    if (input & TurnCW)    m_Yaw += m_RotateSpeed * dt;
    if (input & TurnCCW)   m_Yaw -= m_RotateSpeed * dt;
    if (input & LookUp)    m_Pitch += m_RotateSpeed * dt;
    if (input & LookDown)  m_Pitch -= m_RotateSpeed * dt;

    m_Pitch = glm::clamp(m_Pitch, -glm::radians(89.0f), glm::radians(89.0f));

    // 平移：前/后沿相机局部 forward，左/右沿相机局部 right（始终水平），
    // 上/下沿世界 Y 轴（电梯式垂直移动，不随俯仰倾斜）
    glm::vec3 forward, right, up;
    ComputeBasis(forward, right, up);

    glm::vec3 move(0.0f);
    if (input & MoveForward) move += forward;
    if (input & MoveBack)    move -= forward;
    if (input & MoveRight)   move += right;
    if (input & MoveLeft)    move -= right;
    if (input & MoveUp)      move += glm::vec3(0.0f, 1.0f, 0.0f);
    if (input & MoveDown)    move -= glm::vec3(0.0f, 1.0f, 0.0f);

    // 归一化使斜向移动不快于正向（W+D 同时按住时）
    if (glm::dot(move, move) > 0.0f) {
        m_Position += glm::normalize(move) * m_MoveSpeed * dt;
    }

    ApplyToCamera();
}

// 左手系球坐标基向量：yaw=0, pitch=0 时 forward=(0,0,1)、right=(1,0,0)、up=(0,1,0)
void CameraController::ComputeBasis(glm::vec3& forward, glm::vec3& right, glm::vec3& up) const {
    const float cy = glm::cos(m_Yaw);
    const float sy = glm::sin(m_Yaw);
    const float cp = glm::cos(m_Pitch);
    const float sp = glm::sin(m_Pitch);

    forward = glm::vec3(sy * cp, sp, cy * cp);
    right = glm::vec3(cy, 0.0f, -sy);
    up = glm::cross(forward, right);
}

void CameraController::ApplyToCamera() {
    glm::vec3 forward, right, up;
    ComputeBasis(forward, right, up);
    m_Camera.LookAt(m_Position, m_Position + forward, up);
}

} // namespace nvrhi_lab
