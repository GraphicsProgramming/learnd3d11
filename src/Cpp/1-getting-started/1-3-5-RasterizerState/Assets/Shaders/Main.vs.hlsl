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

cbuffer PerApplication
{
    column_major matrix ProjectionMatrix;
}

cbuffer PerFrame
{
    column_major matrix ViewMatrix;
}

cbuffer PerObject
{
    column_major matrix WorldMatrix;
}

VSOutput Main(VSInput input)
{
    const matrix modelViewProjection = mul(WorldMatrix, mul(ViewMatrix, ProjectionMatrix));

    VSOutput output = (VSOutput)0;
    output.Position = mul(modelViewProjection, float4(input.Position, 1.0f));
    output.Color = input.Color;
    output.Uv = input.Uv;
    return output;
}
