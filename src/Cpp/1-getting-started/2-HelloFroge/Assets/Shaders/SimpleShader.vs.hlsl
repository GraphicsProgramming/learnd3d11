struct VertexInput
{
    float3 Position : POSITION;
    float3 Color : COLOR;
    float2 Uv : TEXCOORD;
};

struct PixelInput
{
    float4 Position : SV_Position;
    float4 Color : COLOR;
    float2 Uv : TEXCOORD;
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

PixelInput Main(VertexInput input)
{
    PixelInput output = (PixelInput)0;

    const matrix modelViewProjection = mul(ProjectionMatrix, mul(ViewMatrix, WorldMatrix));
    output.Position = mul(modelViewProjection, float4(input.Position, 1.0f));
    output.Color = float4(input.Color, 1.0f);
    output.Uv = input.Uv;

    return output;
}
