#include "shaders_shared/camera_cbuffer.hlsli"
#include "shaders_shared/object_cbuffer.hlsli"

// The scene's single vertex shader: position/normal -> world space -> clip space.
// Per-object variation (transform) comes from the shared array, selected by
// the push-constant object index.

struct VSInput
{
    float3 position : POSITION;
    float3 normal   : NORMAL;
};

struct VSOutput
{
    float4 position : SV_Position;
    float3 worldPos : WORLD_POS;
    float3 normal   : WORLD_NORMAL;
};

VSOutput main(VSInput input)
{
    VSOutput output;
    ObjectConstants obj = g_Objects[g_ObjectIndex];
    float4 world = mul(obj.ModelMatrix, float4(input.position, 1.0));
    output.position = mul(ViewProjectionMatrix, world);
    output.worldPos = world.xyz;
    // uniform scale assumed - rotate the normal by the upper-left 3x3
    output.normal = normalize(mul((float3x3)obj.ModelMatrix, input.normal));
    return output;
}
