#include "shaders_shared/object_cbuffer.hlsli"

// The scene's single pixel shader: material variation (BaseColor / lighting
// toggle / grid overlay) all comes from the shared constants array, selected
// by the push-constant object index - no per-object-type shaders.

struct PSInput
{
    float4 position : SV_Position;
    float3 worldPos : WORLD_POS;
    float3 normal   : WORLD_NORMAL;
};

// Grid lines with fixed world-space width (no derivatives; stable on every backend)
float GridMask(float coord, float spacing, float halfWidth)
{
    // distance to the nearest multiple of spacing
    float dist = abs(frac(coord / spacing + 0.5) - 0.5) * spacing;
    return step(dist, halfWidth);  // dist <= halfWidth -> 1
}

float4 main(PSInput input) : SV_Target0
{
    ObjectConstants obj = g_Objects[g_ObjectIndex];
    float3 color = obj.BaseColor.rgb;

    if (obj.LightingMode > 0.5)
    {
        // Lambert: directional light from the upper camera side + ambient
        // (the dark side stays visible)
        float3 N = normalize(input.normal);
        float3 L = normalize(float3(-0.5, 1.0, -0.6));
        float diffuse = max(dot(N, L), 0.0);
        color *= (0.35 + 0.65 * diffuse);
    }

    if (obj.GridEnabled > 0.5)
    {
        float minor = max(GridMask(input.worldPos.x, 1.0, 0.012), GridMask(input.worldPos.y, 1.0, 0.012));
        float major = max(GridMask(input.worldPos.x, 5.0, 0.028), GridMask(input.worldPos.y, 5.0, 0.028));
        color = lerp(color, float3(0.32, 0.38, 0.48), minor);
        color = lerp(color, float3(0.55, 0.65, 0.78), major);
    }

    return float4(color, 1.0);
}
