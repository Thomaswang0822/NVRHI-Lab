#pragma once

#include <glm/glm.hpp>

namespace nvrhi_lab {

// Per-object shader constants (b1)。GPU layout mirrored in
// src/shaders/shaders_shared/object_cbuffer.hlsli。
// 每帧每个 draw 更新一次：变换 + 材质参数一起走这一个缓冲
struct ObjectConstants {
    glm::mat4 ModelMatrix{1.0f};
    glm::vec4 BaseColor{1.0f};
    float LightingMode = 1.0f;  // 0 = unlit, 1 = Lambert
    float GridEnabled = 0.0f;   // 1 = 校准网格叠加
    float Pad[2]{};
};

} // namespace nvrhi_lab
