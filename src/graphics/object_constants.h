#pragma once

#include <glm/glm.hpp>

namespace nvrhi_lab {

// Per-object shader constants (b1 数组里的一个元素)。GPU layout mirrored in
// src/shaders/shaders_shared/object_cbuffer.hlsli。
// 所有物体共用一个数组 CB，加载时一次性写入（内容静态）；
// 每个 draw 用 push constants（b2）传自己的数组下标
struct ObjectConstants {
    glm::mat4 ModelMatrix{1.0f};
    glm::vec4 BaseColor{1.0f};
    float LightingMode = 1.0f;  // 0 = unlit, 1 = Lambert
    float GridEnabled = 0.0f;   // 1 = 校准网格叠加
    float Pad[2]{};
};

// 整个共享数组的上传镜像，必须与 shader 里的 g_Objects[64] 布局一致
//（96B × 64 = 6144B，低于三后端 CB 大小保证下限）
struct ObjectConstantsArray {
    static constexpr uint32_t MaxObjects = 64;
    ObjectConstants Objects[MaxObjects]{};
};

} // namespace nvrhi_lab
