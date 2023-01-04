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

cbuffer PerFrame : register(b0)
{
    row_major matrix viewprojection;
};

cbuffer PerObject : register(b1)
{
    row_major matrix modelmatrix;
};

VSOutput Main(VSInput input)
{
    VSOutput output = (VSOutput)0;

    matrix world = mul(modelmatrix, viewprojection);
    output.Position = mul(world, float4(input.Position, 1.0));
    output.Color = input.Color;
    output.Uv = input.Uv;
    return output;
}
