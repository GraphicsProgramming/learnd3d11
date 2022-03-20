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

cbuffer PerApplication : register(b0)
{
    matrix ProjectionMatrix;
}

cbuffer PerFrame : register(b1)
{
    matrix ViewMatrix;
}

cbuffer PerObject : register(b2)
{
    matrix WorldMatrix;
}

VSOutput Main(VSInput input)
{
    const matrix modelViewProjection = mul(ProjectionMatrix, mul(ViewMatrix, WorldMatrix));

    VSOutput output = (VSOutput)0;
    output.Position = mul(modelViewProjection, float4(input.Position, 1.0f));
    output.Color = input.Color;
    output.Uv = input.Uv;
    return output;
}
