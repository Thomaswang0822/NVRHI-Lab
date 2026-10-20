#ifndef OBJECT_CBUFFER_HLSLI
#define OBJECT_CBUFFER_HLSLI

// Per-object constants (b1) - CPU struct mirrored in
// src/graphics/object_constants.h
cbuffer ObjectConstants : register(b1)
{
    float4x4 ModelMatrix;
    float4 BaseColor;    // rgb = material base color
    float LightingMode;  // 0 = unlit, 1 = Lambert
    float GridEnabled;   // 1 = calibration grid overlay
    float2 _Pad;
};

#endif
