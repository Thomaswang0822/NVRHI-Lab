#pragma once

#include <glm/glm.hpp>

namespace nvrhi_lab {

// 材质 = 纯数据参数。全场景只有一条管线、一对 shader，
// 物体的外观差异全部通过 ObjectConstants（BaseColor 等）表达
struct Material {
    glm::vec3 baseColor{0.8f, 0.8f, 0.8f};
    bool lit = true;    // Lambert 光照；false = unlit
    bool grid = false;  // 校准网格叠加（背景墙用）
};

} // namespace nvrhi_lab
