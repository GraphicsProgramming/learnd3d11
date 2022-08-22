struct VSInput
{
    float3 position: POSITION;
    float3 color: COLOR0;
    float2 uv: TEXCOORD0;
};

struct VSOutput
{
    float4 position: SV_Position;
    float3 color: COLOR0;
    float2 uv: TEXCOORD0;
};

VSOutput Main(VSInput input)
{
    VSOutput output = (VSOutput)0;
    output.position = float4(input.position, 1.0);
    output.color = input.color;
    output.uv = input.uv;
    return output;
}