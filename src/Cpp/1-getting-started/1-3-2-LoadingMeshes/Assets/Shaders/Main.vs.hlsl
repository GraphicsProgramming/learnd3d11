struct VSInput
{
    float3 Position: POSITION;
    float3 Color: COLOR0;
    float2 Uv: UV0;
};

struct VSOutput
{
    float4 Position: SV_POSITION;
    float3 Color: COLOR0;
    float2 Uv: UV0;
};

cbuffer PerApplication
{
    matrix ProjectionMatrix;
}

cbuffer PerFrame
{
    matrix ViewMatrix;
}

cbuffer PerObject
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
