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

# Vulkan-Headers：版本对齐 NVRHI@5410046 的嵌套 pin（v1.4.328）。
# 先于 NVRHI 提供 target，NVRHI 的 `NOT TARGET Vulkan-Headers` 守卫会
# 跳过其 FetchContent 后必为空的嵌套 submodule 目录
FetchContent_Declare(Vulkan-Headers
    GIT_REPOSITORY https://gh-proxy.com/https://github.com/KhronosGroup/Vulkan-Headers.git
    GIT_TAG        v1.4.328
    GIT_SHALLOW    TRUE)
FetchContent_MakeAvailable(Vulkan-Headers)

if(WIN32)
    set(NVRHI_WITH_DX11 ON)
    set(NVRHI_WITH_DX12 ON)
    # D3D12 后端依赖，对齐 NVRHI 嵌套 pin（裸 SHA 不能加 GIT_SHALLOW）
    FetchContent_Declare(DirectX-Headers
        GIT_REPOSITORY https://gh-proxy.com/https://github.com/microsoft/DirectX-Headers.git
        GIT_TAG        d873b344dc540898868697245f100c2a67fe68d9)
    FetchContent_MakeAvailable(DirectX-Headers)
endif()

set(NVRHI_WITH_VULKAN ON)
set(NVRHI_WITH_VALIDATION ON)
FetchContent_Declare(NVRHI
    GIT_REPOSITORY https://gh-proxy.com/https://github.com/NVIDIA-RTX/NVRHI.git
    GIT_TAG        54100464714de88a5a5059d25808f5ccb914ad7d
    GIT_PROGRESS   TRUE)
FetchContent_MakeAvailable(NVRHI)

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
    # 版本不一致会触发 getVkHeaderVersion() 断言）。链接 Vulkan-Headers target
    # 即可获得 FetchContent 拉取的 v1.4.328 头文件路径；
    # 必须排在 Vulkan::Vulkan 之前，确保 v328 优先于系统 SDK 的 v363
    target_link_libraries(${PROJECT_NAME} PRIVATE
        wx::core wx::base
        nvrhi nvrhi_vk Vulkan-Headers Vulkan::Vulkan
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
