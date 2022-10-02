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
    output.position = mul(modelViewProjection, float4(input.position, 1.0));
    output.color = input.color;
    output.uv = input.uv;
    return output;
}