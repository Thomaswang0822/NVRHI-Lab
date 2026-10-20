#ifndef OBJECT_CBUFFER_HLSLI
#define OBJECT_CBUFFER_HLSLI

// Per-object constants (b1) - a shared array with one entry per object.
// The current draw's index arrives via push constants (b2).
// CPU mirror: src/graphics/object_constants.h
struct ObjectConstants
{
    float4x4 ModelMatrix;
    float4 BaseColor;    // rgb = material base color
    float LightingMode;  // 0 = unlit, 1 = Lambert
    float GridEnabled;   // 1 = calibration grid overlay
    float2 _Pad;
};

cbuffer ObjectConstantsBuffer : register(b1)
{
    ObjectConstants g_Objects[64];
}

// Per-draw object index, passed via NVRHI push constants.
// Two compile paths share one name:
// - SPIR-V (Vulkan): DXC only accepts [[vk::push_constant]] on a global
//   struct variable, never on a cbuffer; the build injects -DSPIRV=1 for
//   Vulkan targets. The block maps to the push-constant range in the
//   pipeline layout (offset 0, 4 bytes).
// - D3D (FXC / DXC-DXBC): a regular cbuffer at b2, which NVRHI backs with
//   an internal constant buffer (D3D11) or root constants (D3D12).
#ifdef SPIRV
struct cObjectIndex_t { uint g_ObjectIndex; };
[[vk::push_constant]] cObjectIndex_t PC;
#define g_ObjectIndex PC.g_ObjectIndex
#else
cbuffer cObjectIndex : register(b2)
{
    uint g_ObjectIndex;
};
#endif

#endif
