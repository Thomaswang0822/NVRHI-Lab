#pragma once

#include <nvrhi/nvrhi.h>
#include <cstdint>
#include <string>
#include <vector>
#include "device_manager.h"

namespace nvrhi_lab {

// 按后端加载编译好的 shader 字节码（bin/shaders/<backend>/<name>.<ext>）。
// 查找基于可执行文件所在目录而非 CWD，任何启动方式都能定位
std::vector<uint8_t> LoadShaderFile(const std::string& filename, GraphicsBackend backend);

// nvrhi::ShaderHandle 的便捷封装：加载字节码并创建对应类型的 shader，
// 失败返回 nullptr 并记日志
nvrhi::ShaderHandle CreateShaderFromFile(nvrhi::IDevice* device, const std::string& name,
                                         nvrhi::ShaderType shaderType, GraphicsBackend backend);

} // namespace nvrhi_lab
