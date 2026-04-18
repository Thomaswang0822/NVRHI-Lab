#ifndef CAMERA_CBUFFER_GLSL
#define CAMERA_CBUFFER_GLSL

layout(std140, binding = 0) uniform CameraConstants
{
    mat4 ViewMatrix;
    mat4 ProjectionMatrix;
    mat4 ViewProjectionMatrix;
    vec3 CameraPosition;
    float _Padding;
};

#endif
