#include "shader_utils.h"
#include "../utils/logging.h"

#include <fstream>
#include <filesystem>

#ifdef _WIN32
    #include <windows.h>
#elif defined(__APPLE__)
    #include <mach-o/dyld.h>
#else
    #include <unistd.h>
#endif

namespace nvrhi_lab {

namespace {

// 可执行文件所在目录（bin/）。shader 查找基于它而非 CWD，
// 使任何启动方式（VSCode launch / terminal / 双击）都能定位 bin/shaders/
std::filesystem::path ExecutableDir()
{
#ifdef _WIN32
    wchar_t path[MAX_PATH] = {};
    GetModuleFileNameW(nullptr, path, MAX_PATH);
    return std::filesystem::path(path).parent_path();
#elif defined(__APPLE__)
    uint32_t size = 0;
    _NSGetExecutablePath(nullptr, &size);
    std::vector<char> buffer(size);
    _NSGetExecutablePath(buffer.data(), &size);
    return std::filesystem::path(buffer.data()).lexically_normal().parent_path();
#else
    return std::filesystem::read_symlink("/proc/self/exe").lexically_normal().parent_path();
#endif
}

std::string GetShaderSubdirectory(GraphicsBackend backend) {
    const char* subDir = "";
    switch (backend) {
        case GraphicsBackend::D3D11:  subDir = "d3d11";  break;
        case GraphicsBackend::D3D12:  subDir = "d3d12";  break;
        case GraphicsBackend::Vulkan: subDir = "vulkan"; break;
        default: break;
    }
    return (ExecutableDir() / "shaders" / subDir).string() + "/";
}

std::string GetShaderExtension(GraphicsBackend backend) {
    switch (backend) {
        case GraphicsBackend::D3D11:
        case GraphicsBackend::D3D12:
            return ".cso";
        case GraphicsBackend::Vulkan:
            return ".spv";
        default:
            return ".cso";
    }
}

} // anonymous namespace

std::vector<uint8_t> LoadShaderFile(const std::string& filename, GraphicsBackend backend) {
    std::string shaderPath = GetShaderSubdirectory(backend) + filename;
    std::ifstream file(shaderPath, std::ios::binary | std::ios::ate);

    if (!file.is_open()) {
        LogError("Failed to open shader file: " + shaderPath);
        return {};
    }

    size_t fileSize = static_cast<size_t>(file.tellg());
    file.seekg(0);

    std::vector<uint8_t> buffer(fileSize);
    file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
    file.close();

    return buffer;
}

nvrhi::ShaderHandle CreateShaderFromFile(nvrhi::IDevice* device, const std::string& name,
                                         nvrhi::ShaderType shaderType, GraphicsBackend backend) {
    auto bytecode = LoadShaderFile(name + GetShaderExtension(backend), backend);
    if (bytecode.empty()) {
        return nullptr;
    }

    const char* typeDebug = shaderType == nvrhi::ShaderType::Vertex ? "VS" : "PS";
    nvrhi::ShaderHandle shader = device->createShader(
        nvrhi::ShaderDesc().setShaderType(shaderType).setDebugName(name + typeDebug),
        bytecode.data(), bytecode.size());
    if (!shader) {
        LogError("Failed to create shader: " + name);
    }
    return shader;
}

} // namespace nvrhi_lab
