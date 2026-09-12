struct VSInput {
    float2 in_pos;
    float4 in_color;
}

struct VSOutput {
    float4 pos : SV_Position;
    float4 color;
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
    float4 color = input.color;
    return color;
}