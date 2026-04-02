# Shader compilation configuration
# Uses Visual Studio's native HLSL compiler

set(SHADER_OUTPUT_DIR ${CMAKE_SOURCE_DIR}/bin/shaders)

# Shader files (explicit listing)
set(SHADERS
    src/shaders/triangle.vs.hlsl
    src/shaders/triangle.ps.hlsl
)

function(setup_shader_compilation TARGET_NAME)
    # Create output directory
    file(MAKE_DIRECTORY ${SHADER_OUTPUT_DIR})

    # Configure vertex shader for VS native compilation
    set_source_files_properties(
        src/shaders/triangle.vs.hlsl
        PROPERTIES
            VS_SHADER_TYPE "Vertex"
            VS_SHADER_MODEL "6.4"
            VS_SHADER_ENTRYPOINT "main"
            VS_SHADER_ENABLE_DEBUG "1"
            VS_SHADER_OBJECT_FILE_NAME "shaders/triangle.vs.cso"
    )

    # Configure pixel shader for VS native compilation
    set_source_files_properties(
        src/shaders/triangle.ps.hlsl
        PROPERTIES
            VS_SHADER_TYPE "Pixel"
            VS_SHADER_MODEL "6.4"
            VS_SHADER_ENTRYPOINT "main"
            VS_SHADER_ENABLE_DEBUG "1"
            VS_SHADER_OBJECT_FILE_NAME "shaders/triangle.ps.cso"
    )
endfunction()
