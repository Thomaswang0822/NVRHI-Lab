# Shader compilation configuration
# D3D12: Uses Visual Studio's native HLSL compiler (SM 6.4)
# D3D11: Uses FXC compiler via CMake custom commands (SM 5.1)
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
set(SHADER_ENTRYPOINT "main")

find_program(FXC_COMPILER fxc
    HINTS 
        "$ENV{WindowsSdkDir}bin/$ENV{WindowsSDKVersion}x64"
        "$ENV{WindowsSdkDir}bin/x64"
    PATHS
        "C:/Program Files (x86)/Windows Kits/10/bin/10.0.26100.0/x64"
        "C:/Program Files (x86)/Windows Kits/10/bin/10.0.22621.0/x64"
        "C:/Program Files (x86)/Windows Kits/10/bin/10.0.19041.0/x64"
)

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

function(setup_shader_compilation TARGET_NAME)
    set(ALL_SHADERS "")
    set(ALL_D3D11_OUTPUTS "")

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
        endforeach()
    endforeach()

    set(SHADERS ${ALL_SHADERS} PARENT_SCOPE)
    
    if(ALL_D3D11_OUTPUTS)
        add_custom_target(Shaders_D3D11 ALL DEPENDS ${ALL_D3D11_OUTPUTS})
        add_dependencies(${TARGET_NAME} Shaders_D3D11)
    endif()
endfunction()
