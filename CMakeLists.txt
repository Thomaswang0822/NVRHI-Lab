cmake_minimum_required(VERSION 3.20)
project(NVRHI-Lab VERSION 0.1.6 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# macOS 的 .mm（Objective-C++）源文件需要启用 OBJCXX 语言
if(APPLE)
    enable_language(OBJCXX)
endif()

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

add_compile_definitions(NOMINMAX)

include(FetchContent)

# wxWidgets：rolling，跟踪 master tip（首次 populate 时解析）。
# UPDATE_DISCONNECTED：跳过每次 configure 的 update step——branch 类 GIT_TAG
# 默认每次 configure 都会 fetch 主仓库 + 全部嵌套 submodule（wx 的 .gitmodules
# 指向 github.com 直连，被墙时 75s 超时×13 个子模块）。想刷新 master：
# 删掉 build/_deps/wxwidgets-{src,subbuild,build} 再重新 configure
set(wxBUILD_SHARED OFF)
FetchContent_Declare(wxWidgets
    GIT_REPOSITORY https://gh-proxy.com/https://github.com/wxWidgets/wxWidgets.git
    GIT_TAG        master
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
    UPDATE_DISCONNECTED TRUE)
FetchContent_MakeAvailable(wxWidgets)

# NVRHI 的开关用 CACHE 变量设置：NVRHI 内部用 option() 声明同名开关，而其
# cmake_minimum_required(3.11) 让 CMP0077 保持旧语义——option() 会清除同名
# 普通变量（之前的 configure warning 由此而来）。CACHE 变量不受 option()
# 影响；上面的镜像 URL 变量即用此法注入，已验证可靠
if(WIN32)
    set(NVRHI_WITH_DX11 ON CACHE BOOL "")
    set(NVRHI_WITH_DX12 ON CACHE BOOL "")
endif()
set(NVRHI_WITH_VULKAN ON CACHE BOOL "")
set(NVRHI_WITH_VALIDATION ON CACHE BOOL "")

# NVRHI：rolling，跟踪 main tip（首次 populate 时解析；UPDATE_DISCONNECTED
# 跳过每次 configure 的 fetch，刷新 = 删 build/_deps/nvrhi-* 再 configure）。
# 新版 NVRHI 无嵌套 submodule，自己用 FetchContent 拉取 Vulkan-Headers /
# DirectX-Headers，仓库地址用这两个 cache 变量覆盖为镜像（默认直连 github）
set(NVRHI_VULKAN_HEADERS_GIT_REPOSITORY "https://gh-proxy.com/https://github.com/KhronosGroup/Vulkan-Headers.git" CACHE STRING "")
set(NVRHI_DIRECTX_HEADERS_GIT_REPOSITORY "https://gh-proxy.com/https://github.com/microsoft/DirectX-Headers.git" CACHE STRING "")
FetchContent_Declare(NVRHI
    GIT_REPOSITORY https://gh-proxy.com/https://github.com/NVIDIA-RTX/NVRHI.git
    GIT_TAG        main
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
    UPDATE_DISCONNECTED TRUE)
FetchContent_MakeAvailable(NVRHI)

# GLM：header-only 数学库（Phase 2 引入）。固定版本 tag，不用 GIT_SHALLOW
# （shallow 与特定 tag/SHA 不兼容——见 project memory 教训）；GLM 仓库小，
# 全量 clone 代价可忽略。自带 CMake，提供 glm::glm target
FetchContent_Declare(glm
    GIT_REPOSITORY https://gh-proxy.com/https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1)
FetchContent_MakeAvailable(glm)

find_package(Vulkan REQUIRED)

file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.h")

# 非 Windows 平台：排除 D3D 上下文源文件，加入 Objective-C++ 源（macOS Metal surface）
if(NOT WIN32)
    list(FILTER SOURCES EXCLUDE REGEX "d3d1[12]_context\\.(cpp|h)$")
    file(GLOB_RECURSE OBJCXX_SOURCES "src/*.mm")
    list(APPEND SOURCES ${OBJCXX_SOURCES})
endif()

# Include shader compilation module
include(cmake/shaders.cmake)

if(WIN32)
    add_executable(${PROJECT_NAME} WIN32 ${SOURCES})
    setup_shader_compilation(${PROJECT_NAME})
    # Add shaders to target so they appear in VS Solution Explorer
    target_sources(${PROJECT_NAME} PRIVATE ${SHADERS})
    source_group("Shader Files" FILES ${SHADERS})

    target_link_libraries(${PROJECT_NAME} PRIVATE
        wx::core wx::base
        nvrhi nvrhi_d3d11 nvrhi_d3d12 nvrhi_vk Vulkan-Headers Vulkan::Vulkan
        glm::glm
    )

    target_link_libraries(${PROJECT_NAME} PRIVATE
        dxgi.lib
        d3d11.lib
        d3d12.lib
    )

    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX-)
else()
    add_executable(${PROJECT_NAME} ${SOURCES})
    setup_shader_compilation(${PROJECT_NAME})

    # 必须与 NVRHI 用同一套 Vulkan 头文件（共享 VULKAN_HPP_DEFAULT_DISPATCHER，
    # 版本不一致会触发 getVkHeaderVersion() 断言）。Vulkan-Headers target 由
    # NVRHI 内部 FetchContent 提供；必须排在 Vulkan::Vulkan 之前，
    # 确保其头文件优先于系统 SDK 的高版本
    target_link_libraries(${PROJECT_NAME} PRIVATE
        wx::core wx::base
        nvrhi nvrhi_vk Vulkan-Headers Vulkan::Vulkan
        glm::glm
    )

    if(APPLE)
        # macos_surface.mm 用到 AppKit/QuartzCore(CAMetalLayer)/Metal
        target_link_libraries(${PROJECT_NAME} PRIVATE
            "-framework AppKit"
            "-framework QuartzCore"
            "-framework Metal"
        )
        set_source_files_properties(src/graphics/platform/macos_surface.mm PROPERTIES
            COMPILE_FLAGS "-fobjc-arc"
        )
    endif()
endif()
