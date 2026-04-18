#pragma once

#include "../utils/math_types.h"
#include <glm/glm.hpp>

namespace nvrhi_lab {

struct CameraConstants {
    glm::mat4 ViewMatrix{1.0f};
    glm::mat4 ProjectionMatrix{1.0f};
    glm::mat4 ViewProjectionMatrix{1.0f};
    glm::vec3 CameraPosition{0.0f};
    float _Padding = 0.0f;
};

}
