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
    row_major matrix ProjectionMatrix;
}

cbuffer PerFrame : register(b1)
{
    row_major matrix ViewMatrix;
}

cbuffer PerObject : register(b2)
{
    row_major matrix WorldMatrix;
}

VSOutput Main(VSInput input)
{
    const matrix modelViewProjection = mul(WorldMatrix, mul(ViewMatrix, ProjectionMatrix));

    VSOutput output = (VSOutput)0;
    output.Position = mul(float4(input.Position, 1.0f), modelViewProjection);
    output.Color = input.Color;
    output.Uv = input.Uv;
    return output;
}
