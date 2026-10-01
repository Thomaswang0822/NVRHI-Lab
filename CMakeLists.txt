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

set(wxBUILD_SHARED OFF)
add_subdirectory(thirdparty/wxWidgets)

if(WIN32)
    set(NVRHI_WITH_DX11 ON)
    set(NVRHI_WITH_DX12 ON)
endif()
set(NVRHI_WITH_VULKAN ON)
set(NVRHI_WITH_VALIDATION ON)
add_subdirectory(thirdparty/NVRHI)

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

    # 必须与 NVRHI 使用同一套 Vulkan 头文件（其自带 Vulkan-Headers）编译：
    # NVRHI 与应用共享 VULKAN_HPP_DEFAULT_DISPATCHER，头文件版本不一致会触发
    # getVkHeaderVersion() 断言失败
    target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/thirdparty/NVRHI/thirdparty/Vulkan-Headers/include
    )

    target_link_libraries(${PROJECT_NAME} PRIVATE
        wx::core wx::base
        nvrhi nvrhi_vk Vulkan::Vulkan
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
