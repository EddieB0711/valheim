struct VSIn
{
    float2 position;
    float2 uv;
    float4 color;
}

struct VSOut
{
    float4 position : SV_Position;
    float4 color;
    float2 uv;
}

struct PushConst
{
    float2 scale;
    float2 translate;
}

[vk_push_constant]
PushConst pc;

[shader("vertex")]
VSOut vertMain(VSIn in)
{
    VSOut out;
    out.color = in.color;
    out.uv = in.uv;

    float2 p = mul(in.position, pc.scale + pc.translate);
    out.position = float4(p, 0, 1);

    return out;
}

Sampler2D texture;

[shader("pixel")]
float4 fragMain(VSOut in)
{
    return in.color * texture.Sample(in.uv);
}
