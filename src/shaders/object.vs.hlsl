#include "shaders_shared/camera_cbuffer.hlsli"
#include "shaders_shared/object_cbuffer.hlsli"

// 全场景唯一的顶点着色器：位置/法线 → 世界空间 → 裁剪空间。
// 物体差异（变换）全部来自 ObjectConstants

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
    float4 world = mul(ModelMatrix, float4(input.position, 1.0));
    output.position = mul(ViewProjectionMatrix, world);
    output.worldPos = world.xyz;
    // uniform scale assumed — rotate the normal by the upper-left 3x3
    output.normal = normalize(mul((float3x3)ModelMatrix, input.normal));
    return output;
}
