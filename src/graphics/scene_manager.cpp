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

    if (!LoadDefaultScene(device)) {
        return false;
    }

    // 每物体 CB + binding set，常量一次性写入
    if (!CreateObjectResources(device, cameraBuffer)) {
        return false;
    }

    return true;
}

bool SceneManager::CreatePipeline(nvrhi::IDevice* device, GraphicsBackend backend,
                                  nvrhi::Format colorFormat) {
    // 共享绑定布局：b0 = 相机（VS 用），b1 = 物体常量数组（VS 用矩阵、
    // PS 用材质），b2 = push constants 传当前 draw 的数组下标。
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
            .addItem(nvrhi::BindingLayoutItem::ConstantBuffer(1))
            .addItem(nvrhi::BindingLayoutItem::PushConstants(2, sizeof(uint32_t))));
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

bool SceneManager::CreateObjectResources(nvrhi::IDevice* device, nvrhi::BufferHandle cameraBuffer) {
    // 标准做法：所有物体常量装进一个共享数组 CB（b1，一次创建、
    // 加载时一次性写入）；每个 draw 用 push constants（b2）传自己的
    // 数组下标——push 值随命令流内联提交，不存在共享内存时序问题
    if (m_Objects.empty() || m_Objects.size() > ObjectConstantsArray::MaxObjects) {
        LogError("SceneManager: object count exceeds the constant array capacity");
        return false;
    }

    if (!m_ObjectCB.Create(device, "ObjectConstantArray")) {
        LogError("SceneManager: failed to create object constant buffer");
        return false;
    }

    m_BindingSet = device->createBindingSet(
        nvrhi::BindingSetDesc()
            .addItem(nvrhi::BindingSetItem::ConstantBuffer(0, cameraBuffer))
            .addItem(nvrhi::BindingSetItem::ConstantBuffer(1, m_ObjectCB.GetBuffer()))
            .addItem(nvrhi::BindingSetItem::PushConstants(2, sizeof(uint32_t))),
        m_BindingLayout);
    if (!m_BindingSet) {
        LogError("SceneManager: failed to create binding set");
        return false;
    }

    // 打包整块常量数组，一次性写入（独立 command list 立即执行，模式同 Mesh::Create）
    ObjectConstantsArray array;
    for (size_t i = 0; i < m_Objects.size(); i++) {
        const SceneObject& object = m_Objects[i];
        array.Objects[i].ModelMatrix = object.transform;
        array.Objects[i].BaseColor = glm::vec4(object.material.baseColor, 1.0f);
        array.Objects[i].LightingMode = object.material.lit ? 1.0f : 0.0f;
        array.Objects[i].GridEnabled = object.material.grid ? 1.0f : 0.0f;
    }

    nvrhi::CommandListHandle commandList = device->createCommandList();
    if (!commandList) {
        LogError("SceneManager: failed to create upload command list");
        return false;
    }
    commandList->open();
    m_ObjectCB.Update(commandList, array);
    commandList->close();
    device->executeCommandList(commandList);

    return true;
}

void SceneManager::Render(nvrhi::ICommandList* commandList, const nvrhi::GraphicsState& baseState) {
    for (uint32_t i = 0; i < m_Objects.size(); i++) {
        const Mesh& mesh = m_Meshes[m_Objects[i].meshIndex];

        // 常量数组加载时已写好；这里只提交本 draw 的下标（内联）+ 绑定 + 绘制
        nvrhi::GraphicsState state = baseState;
        state.setPipeline(m_Pipeline)
             .addVertexBuffer(nvrhi::VertexBufferBinding(mesh.GetVertexBuffer(), 0, 0))
             .setIndexBuffer(nvrhi::IndexBufferBinding(mesh.GetIndexBuffer(), nvrhi::Format::R32_UINT, 0))
             .addBindingSet(m_BindingSet);

        commandList->setGraphicsState(state);
        commandList->setPushConstants(&i, sizeof(uint32_t));

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
