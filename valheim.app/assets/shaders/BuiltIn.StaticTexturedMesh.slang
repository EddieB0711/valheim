import "BuiltIn.PerFrameData.module";

struct VSInput
{
    float3 position;
    float2 texCoords;
    float3 normals;
    float3 color;
}

struct VSOutput
{
    float4 position : SV_Position;
    float2 texCoords;
    float3 normals;
    float3 color;
    float3 fragPos;
}

Sampler2D texture;

[shader("vertex")]
VSOutput vertMain(VSInput input)
{
    float4x4 projection = mul(frameData.projection, frameData.view);
    float4 model = mul(frameData.model, float4(input.position, 1.0));
    float4 position = mul(projection, model);

    VSOutput output;

    output.texCoords = input.texCoords;
    output.normals = input.normals;
    output.color = input.color;
    output.position = position;
    output.fragPos = mul(frameData.model, float4(input.position, 1.0)).xyz;

    return output;
}

[shader("pixel")]
float4 fragMain(VSOutput input) : SV_TARGET
{
    float3 lightColor = float3(1.0, 1.0, 1.0);
    float3 lightPos = float3(-4.8, 2.3, 0.2);

    float ambientStrength = 0.1;
    float3 ambient = mul(ambientStrength, lightColor);
    float3 norm = normalize(input.normals);
    float3 lightDir = normalize(lightPos - input.fragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    float3 diffuse = mul(diff, lightColor);
    float3 result = mul(ambient + diffuse, input.color);
    return texture.Sample(input.texCoords) * float4(result, 1.0);
}
