#include "shaders_shared/camera_cbuffer.hlsli"

struct VSInput {
    float3 position : POSITION;
    float4 color : COLOR;
};

struct VSOutput {
    float4 position : SV_Position;
    float4 color : COLOR;
};

VSOutput main(VSInput input) {
    VSOutput output;
    output.position = mul(ViewProjectionMatrix, float4(input.position, 1.0));
    output.color = input.color;
    return output;
}
