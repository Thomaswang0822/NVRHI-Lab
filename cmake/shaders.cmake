# Shader compilation configuration
# Uses Visual Studio's native HLSL compiler
#
# Naming Convention:
#   *.vs.hlsl - Vertex shader
#   *.ps.hlsl - Pixel shader
#   *.cs.hlsl - Compute shader
#   *.gs.hlsl - Geometry shader
#   *.hs.hlsl - Hull shader (tessellation)
#   *.ds.hlsl - Domain shader (tessellation)

set(SHADER_OUTPUT_DIR ${CMAKE_SOURCE_DIR}/bin/shaders)
set(SHADER_MODEL "6.4")
set(SHADER_ENTRYPOINT "main")

function(_configure_shader SHADER_PATH SHADER_TYPE)
    # Get full filename (NAME_WE would strip ".vs.hlsl" entirely, losing shader type)
    get_filename_component(NAME ${SHADER_PATH} NAME)
    # Remove only .hlsl to preserve ".vs"/".ps" for output filename
    string(REGEX REPLACE "\\.hlsl$" "" NAME ${NAME})

    set_source_files_properties(${SHADER_PATH} PROPERTIES
        VS_SHADER_TYPE ${SHADER_TYPE}
        VS_SHADER_MODEL ${SHADER_MODEL}
        VS_SHADER_ENTRYPOINT ${SHADER_ENTRYPOINT}
        VS_SHADER_ENABLE_DEBUG "$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>"
        VS_SHADER_OBJECT_FILE_NAME "shaders/${NAME}.cso"
    )
endfunction()

function(setup_shader_compilation TARGET_NAME)
    file(MAKE_DIRECTORY ${SHADER_OUTPUT_DIR})

    set(ALL_SHADERS "")

    file(GLOB VS_SHADERS "${CMAKE_SOURCE_DIR}/src/shaders/*.vs.hlsl")
    foreach(SHADER ${VS_SHADERS})
        _configure_shader(${SHADER} "Vertex")
        list(APPEND ALL_SHADERS ${SHADER})
    endforeach()

    file(GLOB PS_SHADERS "${CMAKE_SOURCE_DIR}/src/shaders/*.ps.hlsl")
    foreach(SHADER ${PS_SHADERS})
        _configure_shader(${SHADER} "Pixel")
        list(APPEND ALL_SHADERS ${SHADER})
    endforeach()

    file(GLOB CS_SHADERS "${CMAKE_SOURCE_DIR}/src/shaders/*.cs.hlsl")
    foreach(SHADER ${CS_SHADERS})
        _configure_shader(${SHADER} "Compute")
        list(APPEND ALL_SHADERS ${SHADER})
    endforeach()

    file(GLOB GS_SHADERS "${CMAKE_SOURCE_DIR}/src/shaders/*.gs.hlsl")
    foreach(SHADER ${GS_SHADERS})
        _configure_shader(${SHADER} "Geometry")
        list(APPEND ALL_SHADERS ${SHADER})
    endforeach()

    file(GLOB HS_SHADERS "${CMAKE_SOURCE_DIR}/src/shaders/*.hs.hlsl")
    foreach(SHADER ${HS_SHADERS})
        _configure_shader(${SHADER} "Hull")
        list(APPEND ALL_SHADERS ${SHADER})
    endforeach()

    file(GLOB DS_SHADERS "${CMAKE_SOURCE_DIR}/src/shaders/*.ds.hlsl")
    foreach(SHADER ${DS_SHADERS})
        _configure_shader(${SHADER} "Domain")
        list(APPEND ALL_SHADERS ${SHADER})
    endforeach()

    set(SHADERS ${ALL_SHADERS} PARENT_SCOPE)
endfunction()
