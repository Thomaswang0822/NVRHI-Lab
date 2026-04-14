# Shader compilation configuration
# D3D12: Uses Visual Studio's native HLSL compiler (SM 6.4)
# D3D11: Uses FXC compiler via CMake custom commands (SM 5.0)
# Vulkan: Uses DXC compiler with -spirv flag (SM 6.4)
#
# Naming Convention:
#   *.vs.hlsl - Vertex shader
#   *.ps.hlsl - Pixel shader
#   *.cs.hlsl - Compute shader
#   *.gs.hlsl - Geometry shader
#   *.hs.hlsl - Hull shader (tessellation)
#   *.ds.hlsl - Domain shader (tessellation)

set(SHADER_MODEL_D3D12 "6.4")
set(SHADER_MODEL_D3D11 "5_0")
set(SHADER_MODEL_VULKAN "6_4")
set(SHADER_ENTRYPOINT "main")

# Find a compiler from Windows SDK (FXC for D3D11)
# Uses a local variable to avoid scoping issues with find_program
function(_find_windows_sdk_compiler COMPILER_NAME OUTPUT_VAR)
    set(COMPILER_EXE "${COMPILER_NAME}.exe")
    
    # Use a LOCAL variable for the search result
    # This avoids the issue where find_program sets a variable that
    # might already exist in the parent scope with a different value
    set(COMPILER_PATH "COMPILER_PATH-NOTFOUND")
    
    # Method 1: Try environment variables (set by VS Developer Command Prompt)
    if(DEFINED ENV{WindowsSdkDir} AND DEFINED ENV{WindowsSDKVersion})
        file(TO_CMAKE_PATH "$ENV{WindowsSdkDir}" SDK_DIR)
        string(REPLACE "\\" "/" SDK_VERSION "$ENV{WindowsSDKVersion}")
        string(REGEX REPLACE "/$" "" SDK_VERSION "${SDK_VERSION}")
        find_program(COMPILER_PATH ${COMPILER_EXE}
            PATHS "${SDK_DIR}/bin/${SDK_VERSION}/x64"
            NO_DEFAULT_PATH
        )
    endif()
    
    # Method 2: Try Windows Registry (find latest installed SDK)
    # IMPORTANT: We must check for "NOTFOUND" explicitly because:
    # - find_program sets variable to "XXX-NOTFOUND" on failure
    # - "XXX-NOTFOUND" is a non-empty string, so it's TRUTHY in if()
    # - if(NOT ${VAR}) would be FALSE, skipping this fallback!
    if(COMPILER_PATH STREQUAL "COMPILER_PATH-NOTFOUND")
        get_filename_component(WINDOWS_KITS_DIR "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows Kits\\Installed Roots;KitsRoot10]" ABSOLUTE)
        if(EXISTS "${WINDOWS_KITS_DIR}")
            file(GLOB SDK_VERSIONS "${WINDOWS_KITS_DIR}/bin/10.*")
            if(SDK_VERSIONS)
                list(SORT SDK_VERSIONS ORDER DESCENDING)
                list(GET SDK_VERSIONS 0 LATEST_SDK)
                find_program(COMPILER_PATH ${COMPILER_EXE}
                    PATHS "${LATEST_SDK}/x64"
                    NO_DEFAULT_PATH
                )
            endif()
        endif()
    endif()
    
    # Method 3: Fallback to system PATH
    if(COMPILER_PATH STREQUAL "COMPILER_PATH-NOTFOUND")
        find_program(COMPILER_PATH ${COMPILER_EXE})
    endif()
    
    # Return the result to the PARENT scope using the variable NAME passed in
    set(${OUTPUT_VAR} ${COMPILER_PATH} PARENT_SCOPE)
endfunction()

# Find DXC from Vulkan SDK (required for SPIR-V output)
# Windows SDK's DXC does NOT support -spirv flag!
function(_find_vulkan_dxc OUTPUT_VAR)
    set(DXC_PATH "DXC_PATH-NOTFOUND")
    
    # Method 1: Try VULKAN_SDK environment variable
    if(DEFINED ENV{VULKAN_SDK})
        file(TO_CMAKE_PATH "$ENV{VULKAN_SDK}" VULKAN_SDK_PATH)
        find_program(DXC_PATH dxc
            PATHS 
                "${VULKAN_SDK_PATH}/Bin"
                "${VULKAN_SDK_PATH}/bin"
            NO_DEFAULT_PATH
        )
    endif()
    
    # Method 2: Try Windows Registry
    if(DXC_PATH STREQUAL "DXC_PATH-NOTFOUND")
        get_filename_component(VULKAN_SDK_DIR "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Khronos\\Vulkan\\SDKs;Default]" ABSOLUTE)
        if(EXISTS "${VULKAN_SDK_DIR}")
            find_program(DXC_PATH dxc
                PATHS 
                    "${VULKAN_SDK_DIR}/Bin"
                    "${VULKAN_SDK_DIR}/bin"
                NO_DEFAULT_PATH
            )
        endif()
    endif()
    
    # Method 3: Fallback to system PATH
    if(DXC_PATH STREQUAL "DXC_PATH-NOTFOUND")
        find_program(DXC_PATH dxc)
    endif()
    
    set(${OUTPUT_VAR} ${DXC_PATH} PARENT_SCOPE)
endfunction()

# FXC from Windows SDK for D3D11 (DXBC format)
_find_windows_sdk_compiler(fxc FXC_COMPILER)

# DXC from Vulkan SDK for SPIR-V output
# NOTE: Windows SDK's DXC cannot produce SPIR-V!
_find_vulkan_dxc(DXC_COMPILER)
function(_get_shader_target SHADER_TYPE_SUFFIX SHADER_MODEL TARGET)
    if(SHADER_TYPE_SUFFIX STREQUAL "vs")
        set(TARGET_TYPE "vs")
    elseif(SHADER_TYPE_SUFFIX STREQUAL "ps")
        set(TARGET_TYPE "ps")
    elseif(SHADER_TYPE_SUFFIX STREQUAL "cs")
        set(TARGET_TYPE "cs")
    elseif(SHADER_TYPE_SUFFIX STREQUAL "gs")
        set(TARGET_TYPE "gs")
    elseif(SHADER_TYPE_SUFFIX STREQUAL "hs")
        set(TARGET_TYPE "hs")
    elseif(SHADER_TYPE_SUFFIX STREQUAL "ds")
        set(TARGET_TYPE "ds")
    else()
        set(TARGET_TYPE "vs")
    endif()
    set(${TARGET} "${TARGET_TYPE}_${SHADER_MODEL}" PARENT_SCOPE)
endfunction()

function(_configure_shader_d3d12 SHADER_PATH SHADER_TYPE_NAME)
    get_filename_component(NAME ${SHADER_PATH} NAME)
    string(REGEX REPLACE "\\.hlsl$" "" NAME ${NAME})

    set_source_files_properties(${SHADER_PATH} PROPERTIES
        VS_SHADER_TYPE ${SHADER_TYPE_NAME}
        VS_SHADER_MODEL ${SHADER_MODEL_D3D12}
        VS_SHADER_ENTRYPOINT ${SHADER_ENTRYPOINT}
        VS_SHADER_ENABLE_DEBUG "$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>"
        VS_SHADER_OBJECT_FILE_NAME "shaders/d3d12/${NAME}.cso"
    )
endfunction()

function(_compile_shader_d3d11 SHADER_PATH SHADER_TYPE_SUFFIX)
    get_filename_component(NAME ${SHADER_PATH} NAME)
    string(REGEX REPLACE "\\.hlsl$" "" NAME ${NAME})
    
    _get_shader_target(${SHADER_TYPE_SUFFIX} ${SHADER_MODEL_D3D11} TARGET)
    
    set(OUTPUT_DIR "${CMAKE_BINARY_DIR}/shaders/d3d11")
    set(OUTPUT_CSO "${OUTPUT_DIR}/${NAME}.cso")
    set(OUTPUT_PDB "${OUTPUT_DIR}/${NAME}.pdb")
    
    add_custom_command(
        OUTPUT ${OUTPUT_CSO}
        COMMAND ${CMAKE_COMMAND} -E make_directory "${OUTPUT_DIR}"
        COMMAND ${FXC_COMPILER}
            /T ${TARGET}
            /E ${SHADER_ENTRYPOINT}
            /Zi
            /Od
            /Fd "${OUTPUT_PDB}"
            "${SHADER_PATH}"
            /Fo "${OUTPUT_CSO}"
        DEPENDS ${SHADER_PATH}
        COMMENT "Compiling D3D11 shader: ${NAME}"
        VERBATIM
    )
    
    set(D3D11_SHADER_OUTPUT ${OUTPUT_CSO} PARENT_SCOPE)
endfunction()

function(_compile_shader_vulkan SHADER_PATH SHADER_TYPE_SUFFIX)
    get_filename_component(NAME ${SHADER_PATH} NAME)
    string(REGEX REPLACE "\\.hlsl$" "" NAME ${NAME})
    
    _get_shader_target(${SHADER_TYPE_SUFFIX} ${SHADER_MODEL_VULKAN} TARGET)
    
    set(OUTPUT_DIR "${CMAKE_BINARY_DIR}/shaders/vulkan")
    set(OUTPUT_SPV "${OUTPUT_DIR}/${NAME}.spv")
    
    add_custom_command(
        OUTPUT ${OUTPUT_SPV}
        COMMAND ${CMAKE_COMMAND} -E make_directory "${OUTPUT_DIR}"
        COMMAND ${DXC_COMPILER}
            -T ${TARGET}
            -E ${SHADER_ENTRYPOINT}
            -spirv
            -Zi
            -Od
            -fspv-debug=vulkan-with-source
            "${SHADER_PATH}"
            -Fo "${OUTPUT_SPV}"
        DEPENDS ${SHADER_PATH}
        COMMENT "Compiling Vulkan SPIR-V shader: ${NAME}"
        VERBATIM
    )
    
    set(VULKAN_SHADER_OUTPUT ${OUTPUT_SPV} PARENT_SCOPE)
endfunction()

function(setup_shader_compilation TARGET_NAME)
    set(ALL_SHADERS "")
    set(ALL_D3D11_OUTPUTS "")
    set(ALL_VULKAN_OUTPUTS "")

    set(SHADER_TYPES "vs" "ps" "cs" "gs" "hs" "ds")
    set(SHADER_TYPE_NAMES "Vertex" "Pixel" "Compute" "Geometry" "Hull" "Domain")
    
    list(LENGTH SHADER_TYPES TYPE_COUNT)
    math(EXPR TYPE_END "${TYPE_COUNT} - 1")
    
    foreach(I RANGE ${TYPE_END})
        list(GET SHADER_TYPES ${I} SHADER_SUFFIX)
        list(GET SHADER_TYPE_NAMES ${I} SHADER_TYPE_NAME)
        
        file(GLOB SHADERS "${CMAKE_SOURCE_DIR}/src/shaders/*.${SHADER_SUFFIX}.hlsl")
        
        foreach(SHADER ${SHADERS})
            _configure_shader_d3d12(${SHADER} ${SHADER_TYPE_NAME})
            list(APPEND ALL_SHADERS ${SHADER})
            
            if(FXC_COMPILER)
                _compile_shader_d3d11(${SHADER} ${SHADER_SUFFIX})
                list(APPEND ALL_D3D11_OUTPUTS ${D3D11_SHADER_OUTPUT})
            endif()
            
            if(DXC_COMPILER)
                _compile_shader_vulkan(${SHADER} ${SHADER_SUFFIX})
                list(APPEND ALL_VULKAN_OUTPUTS ${VULKAN_SHADER_OUTPUT})
            endif()
        endforeach()
    endforeach()

    set(SHADERS ${ALL_SHADERS} PARENT_SCOPE)
    
    if(ALL_D3D11_OUTPUTS)
        add_custom_target(Shaders_D3D11 ALL DEPENDS ${ALL_D3D11_OUTPUTS})
        add_dependencies(${TARGET_NAME} Shaders_D3D11)
    endif()
    
    if(ALL_VULKAN_OUTPUTS)
        add_custom_target(Shaders_Vulkan ALL DEPENDS ${ALL_VULKAN_OUTPUTS})
        add_dependencies(${TARGET_NAME} Shaders_Vulkan)
    endif()
endfunction()
