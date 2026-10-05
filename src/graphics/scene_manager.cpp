#include "scene_manager.h"
#include "geometry.h"
#include "shader_utils.h"
#include "../utils/logging.h"

namespace nvrhi_lab {

bool SceneManager::Create(nvrhi::IDevice* device, nvrhi::BufferHandle cameraBuffer,
                          GraphicsBackend backend, nvrhi::Format colorFormat) {
    if (!CreatePipeline(device, backend, colorFormat)) {
        return false;
    }

    // 绑定集：b0 = 相机常量（renderer 每帧更新），b1 = 物体常量
    //（本类持有，每个 draw 前更新内容，缓冲本身全场景共用一个）
    m_BindingSet = device->createBindingSet(
        nvrhi::BindingSetDesc()
            .addItem(nvrhi::BindingSetItem::ConstantBuffer(0, cameraBuffer))
            .addItem(nvrhi::BindingSetItem::ConstantBuffer(1, m_ObjectCB.GetBuffer())),
        m_BindingLayout);
    if (!m_BindingSet) {
        LogError("SceneManager: failed to create binding set");
        return false;
    }

    if (!LoadDefaultScene(device)) {
        return false;
    }

    return true;
}

bool SceneManager::CreatePipeline(nvrhi::IDevice* device, GraphicsBackend backend,
                                  nvrhi::Format colorFormat) {
    // 共享绑定布局：b0 = 相机（VS 用），b1 = 物体（VS 用矩阵、PS 用材质）。
    // Vulkan 下 binding offset 归零以匹配 DXC 的 register→binding 映射
    m_BindingLayout = device->createBindingLayout(
        nvrhi::BindingLayoutDesc()
            .setVisibility(nvrhi::ShaderType::Vertex | nvrhi::ShaderType::Pixel)
            .setBindingOffsets(nvrhi::VulkanBindingOffsets()
                .setConstantBufferOffset(0)
                .setShaderResourceOffset(0)
                .setSamplerOffset(0)
                .setUnorderedAccessViewOffset(0))
            .addItem(nvrhi::BindingLayoutItem::ConstantBuffer(0))
            .addItem(nvrhi::BindingLayoutItem::ConstantBuffer(1)));
    if (!m_BindingLayout) {
        LogError("SceneManager: failed to create binding layout");
        return false;
    }

    // 全场景唯一的一对 shader：object.vs / object.ps
    m_VertexShader = CreateShaderFromFile(device, "object.vs", nvrhi::ShaderType::Vertex, backend);
    m_PixelShader = CreateShaderFromFile(device, "object.ps", nvrhi::ShaderType::Pixel, backend);
    if (!m_VertexShader || !m_PixelShader) {
        return false;
    }

    // 单一顶点格式（位置 + 法线）→ 单一 input layout
    nvrhi::VertexAttributeDesc attributes[] = {
        nvrhi::VertexAttributeDesc()
            .setName("POSITION")
            .setFormat(nvrhi::Format::RGB32_FLOAT)
            .setOffset(offsetof(GeometryVertex, position))
            .setElementStride(sizeof(GeometryVertex)),
        nvrhi::VertexAttributeDesc()
            .setName("NORMAL")
            .setFormat(nvrhi::Format::RGB32_FLOAT)
            .setOffset(offsetof(GeometryVertex, normal))
            .setElementStride(sizeof(GeometryVertex)),
    };

    nvrhi::InputLayoutHandle inputLayout = device->createInputLayout(
        attributes, uint32_t(std::size(attributes)), m_VertexShader);
    if (!inputLayout) {
        LogError("SceneManager: failed to create input layout");
        return false;
    }

    // 真实几何体（非测试三角形）：刻意开启背面剔除——
    // 绕序错误直接表现为面消失，tier-1 验收一眼可见。
    // Vulkan 注意：Camera::SetFlipY 通过投影矩阵取反 Y（手性翻转），
    // 所有三角形绕序随之反转，必须同步翻转 front face 定义，
    // 否则 cullBack 剔除的是正面（墙整面消失、立方体只剩内表面）
    nvrhi::RenderState renderState;
    renderState.depthStencilState.enableDepthTest();
    renderState.rasterState.setCullBack();
    if (backend == GraphicsBackend::Vulkan) {
        renderState.rasterState.setFrontCounterClockwise(true);
    }

    nvrhi::FramebufferInfo framebufferInfo;
    framebufferInfo.addColorFormat(colorFormat).setDepthFormat(nvrhi::Format::D32);

    nvrhi::GraphicsPipelineDesc pipelineDesc;
    pipelineDesc.setInputLayout(inputLayout)
                .setVertexShader(m_VertexShader)
                .setRenderState(renderState)
                .setPixelShader(m_PixelShader)
                .addBindingLayout(m_BindingLayout);

    m_Pipeline = device->createGraphicsPipeline(pipelineDesc, framebufferInfo);
    if (!m_Pipeline) {
        LogError("SceneManager: failed to create graphics pipeline");
        return false;
    }

    if (!m_ObjectCB.Create(device, "ObjectConstantBuffer")) {
        LogError("SceneManager: failed to create object constant buffer");
        return false;
    }

    return true;
}

bool SceneManager::LoadDefaultScene(nvrhi::IDevice* device) {
    // "加载器"：今天从程序化定义生成；之后这一段整体替换为
    // 文件解析（.obj / 场景描述），Mesh/SceneObject 接口不变

    Mesh wall;
    if (!wall.Create(device, MakeGridWall(20.0f, 12.0f, 5.0f), "Wall")) {
        return false;
    }
    m_Meshes.push_back(std::move(wall));

    Mesh cube;
    if (!cube.Create(device, MakeCube(1.0f), "Target")) {
        return false;
    }
    m_Meshes.push_back(std::move(cube));

    // 背景墙：unlit + 校准网格（相机在世界中移动时的视觉参照）
    SceneObject wallObject;
    wallObject.meshIndex = 0;
    wallObject.material.baseColor = glm::vec3(0.09f, 0.10f, 0.14f);
    wallObject.material.lit = false;
    wallObject.material.grid = true;
    m_Objects.push_back(wallObject);

    // 靶子：Lambert 光照立方体（Phase 3 在这里写每个靶子自己的 transform）
    SceneObject targetObject;
    targetObject.meshIndex = 1;
    targetObject.material.baseColor = glm::vec3(0.92f, 0.26f, 0.16f);
    targetObject.material.lit = true;
    m_Objects.push_back(targetObject);

    return true;
}

void SceneManager::Render(nvrhi::ICommandList* commandList, const nvrhi::GraphicsState& baseState) {
    for (const SceneObject& object : m_Objects) {
        const Mesh& mesh = m_Meshes[object.meshIndex];

        // 变换 + 材质一起写入共享的 ObjectConstants；同一 command list
        // 上 writeBuffer 与 draw 按提交顺序执行，逐对象更新是安全的
        ObjectConstants constants;
        constants.ModelMatrix = object.transform;
        constants.BaseColor = glm::vec4(object.material.baseColor, 1.0f);
        constants.LightingMode = object.material.lit ? 1.0f : 0.0f;
        constants.GridEnabled = object.material.grid ? 1.0f : 0.0f;
        m_ObjectCB.Update(commandList, constants);

        nvrhi::GraphicsState state = baseState;
        state.setPipeline(m_Pipeline)
             .addVertexBuffer(nvrhi::VertexBufferBinding(mesh.GetVertexBuffer(), 0, 0))
             .setIndexBuffer(nvrhi::IndexBufferBinding(mesh.GetIndexBuffer(), nvrhi::Format::R32_UINT, 0))
             .addBindingSet(m_BindingSet);

        commandList->setGraphicsState(state);

        nvrhi::DrawArguments args;
        args.setVertexCount(mesh.GetIndexCount());
        commandList->drawIndexed(args);
    }
}

void SceneManager::Shutdown() {
    m_Objects.clear();
    m_Meshes.clear();
    m_BindingSet = nullptr;
    m_Pipeline = nullptr;
    m_PixelShader = nullptr;
    m_VertexShader = nullptr;
    m_BindingLayout = nullptr;
}

} // namespace nvrhi_lab
