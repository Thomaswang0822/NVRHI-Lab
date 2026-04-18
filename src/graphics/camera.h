#pragma once

#include "../utils/math_types.h"
#include "camera_constants.h"

namespace nvrhi_lab {

class Camera {
public:
    Camera();

    void SetPosition(const glm::vec3& pos) { m_Position = pos; m_Dirty = true; }
    void SetTarget(const glm::vec3& target) { m_Target = target; m_Dirty = true; }
    void SetUp(const glm::vec3& up) { m_Up = up; m_Dirty = true; }
    
    void SetPerspective(float fovY, float aspect, float nearZ, float farZ);
    void SetAspectRatio(float aspect) { m_Aspect = aspect; m_Dirty = true; }

    const glm::vec3& GetPosition() const { return m_Position; }
    const glm::vec3& GetTarget() const { return m_Target; }
    const glm::vec3& GetUp() const { return m_Up; }

    const glm::mat4& GetViewMatrix();
    const glm::mat4& GetProjectionMatrix();
    const glm::mat4& GetViewProjectionMatrix();

    CameraConstants GetConstants();

    void LookAt(const glm::vec3& eye, const glm::vec3& target, const glm::vec3& up);

private:
    void UpdateMatrices();

    glm::vec3 m_Position{0.0f, 0.0f, -5.0f};
    glm::vec3 m_Target{0.0f, 0.0f, 0.0f};
    glm::vec3 m_Up{0.0f, 1.0f, 0.0f};

    float m_FovY = glm::radians(45.0f);
    float m_Aspect = 16.0f / 9.0f;
    float m_NearZ = 0.1f;
    float m_FarZ = 100.0f;

    glm::mat4 m_ViewMatrix{1.0f};
    glm::mat4 m_ProjectionMatrix{1.0f};
    glm::mat4 m_ViewProjectionMatrix{1.0f};

    bool m_Dirty = true;
};

} // namespace nvrhi_lab
