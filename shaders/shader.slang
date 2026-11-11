struct PushConstants { float2 vport_size; float2 scroll; };
[[vk::push_constant]] PushConstants pc;

struct VSInput {
    float2 rel_pos : POSITION;
    float4 rect : TEXCOORD0;
    float4 in_color : COLOR0;
}

struct VSOutput {
    float4 pos : SV_Position;
    float4 color : COLOR0;
}

[shader("vertex")]
VSOutput vertMain(VSInput input) {
    VSOutput output;

    float2 rect_origin = input.rect.xy;
    float2 rect_size = input.rect.zw;

    float2 rect_position_pixel = input.rel_pos * rect_size + rect_origin;

    float2 rect_abs_position = rect_position_pixel / pc.vport_size * 2.0 - 1.0;

    output.pos = float4(rect_abs_position + pc.scroll, 0.0, 1.0);
    output.color = input.in_color;
    return output;
}

[shader("fragment")]
float4 fragMain(VSOutput input) : SV_Target
{
    float4 color = input.color;
    return color;
}