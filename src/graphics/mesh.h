#pragma once

#include <nvrhi/nvrhi.h>
#include <cstdint>
#include "geometry.h"

namespace nvrhi_lab {

// 通用网格：顶点/索引缓冲 + 索引数。纯 GPU 资源容器——
// 不含管线、材质或变换，那些属于 SceneObject / SceneManager
class Mesh {
public:
    bool Create(nvrhi::IDevice* device, const GeometryData& geometry, const char* debugName);

    const nvrhi::BufferHandle& GetVertexBuffer() const { return m_VertexBuffer; }
    const nvrhi::BufferHandle& GetIndexBuffer() const { return m_IndexBuffer; }
    uint32_t GetIndexCount() const { return m_IndexCount; }

private:
    nvrhi::BufferHandle m_VertexBuffer;
    nvrhi::BufferHandle m_IndexBuffer;
    uint32_t m_IndexCount = 0;
};

} // namespace nvrhi_lab
