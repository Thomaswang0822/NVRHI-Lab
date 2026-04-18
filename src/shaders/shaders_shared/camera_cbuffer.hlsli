#ifndef CAMERA_CBUFFER_HLSLI
#define CAMERA_CBUFFER_HLSLI

cbuffer CameraConstants : register(b0)
{
    float4x4 ViewMatrix;
    float4x4 ProjectionMatrix;
    float4x4 ViewProjectionMatrix;
    float3 CameraPosition;
    float _Padding;
};

#endif
