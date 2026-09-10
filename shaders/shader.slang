struct VSInput {
    float2 in_pos;
    float3 in_color;
}

struct VSOutput {
    float4 pos : SV_Position;
    float3 color;
}

[shader("vertex")]
VSOutput vertMain(VSInput input) {
    VSOutput output;
    output.pos = float4(input.in_pos, 0.0, 1.0);
    output.color = input.in_color;
    return output;
}

[shader("fragment")]
float4 fragMain(VSOutput input) : SV_Target
{
    float3 color = input.in_colorcolor;
    return float4(color, 1.0);
}