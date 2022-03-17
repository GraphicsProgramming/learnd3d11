struct VSInput
{
    float3 Position: POSITION;
    float3 Color: COLOR0;
    float2 Uv: TEXCOORD0;
};

struct VSOutput
{
    float4 Position: SV_Position;
    float3 Color: COLOR0;
    float2 Uv: TEXCOORD0;
};

VSOutput Main(VSInput input)
{
    VSOutput output = (VSOutput)0;
    output.Position = float4(input.Position, 1.0);
    output.Color = input.Color;
    output.Uv = input.Uv;
    return output;
}
