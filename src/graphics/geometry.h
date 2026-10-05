#pragma once

#include <cstdint>
#include <vector>

namespace nvrhi_lab {

// 全场景统一的顶点格式：位置 + 法线（unlit 物体也带法线，
// 保证单一 input layout / 单一管线）
struct GeometryVertex {
    float position[3];
    float normal[3];
};

// 纯几何数据（CPU 侧），与任何 GPU 资源解耦。
// 今天由程序化函数生成，之后可换成 .obj/场景文件解析器——
// 产出相同结构即可，渲染路径不用动
struct GeometryData {
    std::vector<GeometryVertex> vertices;
    std::vector<uint32_t> indices;
};

// 背景墙：XY 平面四边形，法线朝 -Z（默认相机侧）。
// 顶点为从相机侧看去的 左下/右下/右上/左上，
// 索引绕成 clockwise（LH 前面，背面剔除下从背后不可见）
GeometryData MakeGridWall(float width, float height, float z);

// 立方体（中心在原点，全宽 size）：6 面 x 4 顶点（独立顶点，
// 法线 = 面法线），绕序从外侧看 clockwise
GeometryData MakeCube(float size);

} // namespace nvrhi_lab
