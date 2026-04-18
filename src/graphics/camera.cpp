#include "camera.h"

namespace nvrhi_lab {

Camera::Camera() {
    m_Dirty = true;
}

void Camera::SetPerspective(float fovY, float aspect, float nearZ, float farZ) {
    m_FovY = fovY;
    m_Aspect = aspect;
    m_NearZ = nearZ;
    m_FarZ = farZ;
    m_Dirty = true;
}

void Camera::LookAt(const glm::vec3& eye, const glm::vec3& target, const glm::vec3& up) {
    m_Position = eye;
    m_Target = target;
    m_Up = up;
    m_Dirty = true;
}

const glm::mat4& Camera::GetViewMatrix() {
    UpdateMatrices();
    return m_ViewMatrix;
}

const glm::mat4& Camera::GetProjectionMatrix() {
    UpdateMatrices();
    return m_ProjectionMatrix;
}

const glm::mat4& Camera::GetViewProjectionMatrix() {
    UpdateMatrices();
    return m_ViewProjectionMatrix;
}

CameraConstants Camera::GetConstants() {
    UpdateMatrices();
    
    CameraConstants constants;
    constants.ViewMatrix = m_ViewMatrix;
    constants.ProjectionMatrix = m_ProjectionMatrix;
    constants.ViewProjectionMatrix = m_ViewProjectionMatrix;
    constants.CameraPosition = m_Position;
    constants._Padding = 0.0f;
    
    return constants;
}

void Camera::UpdateMatrices() {
    if (!m_Dirty) {
        return;
    }

    m_ViewMatrix = glm::lookAt(m_Position, m_Target, m_Up);
    m_ProjectionMatrix = glm::perspective(m_FovY, m_Aspect, m_NearZ, m_FarZ);
    m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;

    m_Dirty = false;
}

} // namespace nvrhi_lab
