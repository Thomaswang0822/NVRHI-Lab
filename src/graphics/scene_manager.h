#pragma once

#include <nvrhi/nvrhi.h>
#include <glm/glm.hpp>
#include <vector>
#include "constant_buffer.h"
#include "device_manager.h"
#include "material.h"
#include "mesh.h"
#include "object_constants.h"

namespace nvrhi_lab {

// 场景对象 = 纯数据：引用哪个网格 + 材质参数 + 变换。
// 没有虚函数、没有继承——渲染路径完全通用，物体差异只在数据里
struct SceneObject {
    uint32_t meshIndex = 0;
    Material material;
    glm::mat4 transform{1.0f};
};

// 场景系统 = 加载器 + 存储：
//   - LoadDefaultScene 产出 Mesh / SceneObject 列表（今天程序化定义，
//     之后换成文件解析也只改这一处）
//   - 持有全场景唯一的管线 / VS+PS / 绑定布局，以及逐物体的 CB 与绑定集
//   - Render() 逐对象绑定各自 binding set 后 drawIndexed（零 CB 写入）
// BasicRenderer 只管帧级资源（相机、深度缓冲、清屏、present）
class SceneManager {
public:
    // cameraBuffer: 相机常量缓冲（b0，renderer 拥有并每帧更新）;
    // colorFormat: swapchain 颜色格式（管线目标格式）
    bool Create(nvrhi::IDevice* device, nvrhi::BufferHandle cameraBuffer,
                GraphicsBackend backend, nvrhi::Format colorFormat);
    void Render(nvrhi::ICommandList* commandList, const nvrhi::GraphicsState& baseState);
    void Shutdown();

private:
    bool CreatePipeline(nvrhi::IDevice* device, GraphicsBackend backend, nvrhi::Format colorFormat);
    bool LoadDefaultScene(nvrhi::IDevice* device);
    bool CreateObjectResources(nvrhi::IDevice* device, nvrhi::BufferHandle cameraBuffer);

    std::vector<Mesh> m_Meshes;           // 网格存储（SceneObject 按索引引用）
    std::vector<SceneObject> m_Objects;   // 对象存储
    // 每物体一个 CB：内容（变换 + 材质）静态，加载时一次性写入，
    // 帧循环零 writeBuffer
    std::vector<ConstantBuffer<ObjectConstants>> m_ObjectCBs;
    std::vector<nvrhi::BindingSetHandle> m_ObjectBindingSets;  // b0 共享相机 + b1 各自的 CB

    nvrhi::ShaderHandle m_VertexShader;         // 全场景唯一
    nvrhi::ShaderHandle m_PixelShader;
    nvrhi::GraphicsPipelineHandle m_Pipeline;
    nvrhi::BindingLayoutHandle m_BindingLayout;
};

} // namespace nvrhi_lab
