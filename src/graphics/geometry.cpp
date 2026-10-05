#include "geometry.h"

namespace nvrhi_lab {

GeometryData MakeGridWall(float width, float height, float z) {
    const float hw = width * 0.5f;
    const float hh = height * 0.5f;

    GeometryData data;
    data.vertices = {
        { { -hw, -hh, z }, { 0.0f, 0.0f, -1.0f } },  // bottom-left
        { {  hw, -hh, z }, { 0.0f, 0.0f, -1.0f } },  // bottom-right
        { {  hw,  hh, z }, { 0.0f, 0.0f, -1.0f } },  // top-right
        { { -hw,  hh, z }, { 0.0f, 0.0f, -1.0f } },  // top-left
    };
    data.indices = {
        0, 2, 1,   // clockwise triangle
        0, 3, 2,
    };
    return data;
}

GeometryData MakeCube(float size) {
    const float h = size * 0.5f;

    // 每面：4 个顶点位置（从立方体外看去的 左下/右下/右上/左上）+ 面法线
    static constexpr float kFaces[6][4][3] = {
        // -Z（面向默认相机）
        { {-.5f,-.5f,-.5f}, {.5f,-.5f,-.5f}, {.5f,.5f,-.5f}, {-.5f,.5f,-.5f} },
        // +Z
        { {.5f,-.5f,.5f}, {-.5f,-.5f,.5f}, {-.5f,.5f,.5f}, {.5f,.5f,.5f} },
        // -X
        { {-.5f,-.5f,.5f}, {-.5f,-.5f,-.5f}, {-.5f,.5f,-.5f}, {-.5f,.5f,.5f} },
        // +X
        { {.5f,-.5f,-.5f}, {.5f,-.5f,.5f}, {.5f,.5f,.5f}, {.5f,.5f,-.5f} },
        // +Y（顶面）
        { {-.5f,.5f,-.5f}, {.5f,.5f,-.5f}, {.5f,.5f,.5f}, {-.5f,.5f,.5f} },
        // -Y（底面）
        { {-.5f,-.5f,.5f}, {.5f,-.5f,.5f}, {.5f,-.5f,-.5f}, {-.5f,-.5f,-.5f} },
    };
    static constexpr float kFaceNormals[6][3] = {
        { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f, 1.0f },
        { -1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f },  { 0.0f, -1.0f, 0.0f },
    };

    GeometryData data;
    data.vertices.reserve(24);
    data.indices.reserve(36);

    for (uint32_t f = 0; f < 6; ++f) {
        const uint32_t base = uint32_t(data.vertices.size());
        for (uint32_t v = 0; v < 4; ++v) {
            GeometryVertex vertex{};
            for (uint32_t j = 0; j < 3; ++j) {
                vertex.position[j] = kFaces[f][v][j] * size;  // 单位立方体数据 × size
                vertex.normal[j] = kFaceNormals[f][j];
            }
            data.vertices.push_back(vertex);
        }
        // clockwise from outside: (bl, tr, br) + (bl, tl, tr)
        data.indices.insert(data.indices.end(), {
            base + 0, base + 2, base + 1,
            base + 0, base + 3, base + 2,
        });
    }
    return data;
}

} // namespace nvrhi_lab
