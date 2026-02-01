cmake_minimum_required(VERSION 3.20)
project(NVRHI-Lab VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

set(wxBUILD_SHARED OFF)
add_subdirectory(thirdparty/wxWidgets)

set(NVRHI_WITH_DX11 ON)
set(NVRHI_WITH_DX12 ON)
set(NVRHI_WITH_VULKAN ON)
set(NVRHI_WITH_VALIDATION ON)
add_subdirectory(thirdparty/NVRHI)

file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.h")

if(WIN32)
    add_executable(${PROJECT_NAME} WIN32 ${SOURCES})
else()
    add_executable(${PROJECT_NAME} ${SOURCES})
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE
    wx::core wx::base # wxWidgets
    nvrhi nvrhi_d3d11 nvrhi_d3d12 nvrhi_vk
)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX-)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()
