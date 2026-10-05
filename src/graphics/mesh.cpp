#include "mesh.h"
#include "../utils/logging.h"

namespace nvrhi_lab {

bool Mesh::Create(nvrhi::IDevice* device, const GeometryData& geometry, const char* debugName) {
    if (geometry.vertices.empty() || geometry.indices.empty()) {
        LogError(std::string("Mesh: empty geometry: ") + debugName);
        return false;
    }

    m_VertexBuffer = device->createBuffer(
        nvrhi::BufferDesc()
            .setByteSize(geometry.vertices.size() * sizeof(GeometryVertex))
            .setIsVertexBuffer(true)
            .enableAutomaticStateTracking(nvrhi::ResourceStates::VertexBuffer)
            .setDebugName(std::string(debugName) + ".VB"));
    if (!m_VertexBuffer) {
        LogError(std::string("Mesh: failed to create vertex buffer: ") + debugName);
        return false;
    }

    m_IndexBuffer = device->createBuffer(
        nvrhi::BufferDesc()
            .setByteSize(geometry.indices.size() * sizeof(uint32_t))
            .setIsIndexBuffer(true)
            .enableAutomaticStateTracking(nvrhi::ResourceStates::IndexBuffer)
            .setDebugName(std::string(debugName) + ".IB"));
    if (!m_IndexBuffer) {
        LogError(std::string("Mesh: failed to create index buffer: ") + debugName);
        return false;
    }

    m_IndexCount = uint32_t(geometry.indices.size());

    // 一次性上传（初始化阶段，独立 command list 立即执行）
    nvrhi::CommandListHandle commandList = device->createCommandList();
    if (!commandList) {
        LogError("Mesh: failed to create upload command list");
        return false;
    }
    commandList->open();
    commandList->writeBuffer(m_VertexBuffer, geometry.vertices.data(),
                             geometry.vertices.size() * sizeof(GeometryVertex));
    commandList->writeBuffer(m_IndexBuffer, geometry.indices.data(),
                             geometry.indices.size() * sizeof(uint32_t));
    commandList->close();
    device->executeCommandList(commandList);

    return true;
}

} // namespace nvrhi_lab
