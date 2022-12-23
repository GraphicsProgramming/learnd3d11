struct VSInput
{
    float3 Position: POSITION;
    float3 Color: COLOR0;
};

struct VSOutput
{
    float4 Position: SV_Position;
    float3 Color: COLOR0;
};

VSOutput Main(VSInput input)
{
    VSOutput output = (VSOutput)0;
    output.Position = float4(input.Position, 1.0f);
    output.Color = input.Color;
    return output;
}
