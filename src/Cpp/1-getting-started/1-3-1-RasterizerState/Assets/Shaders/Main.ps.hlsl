struct VSOutput
{
    float4 Position: SV_Position;
    float3 Color: COLOR0;
};


float4 Main(VSOutput input): SV_Target
{
    return float4(input.Color, 1.0f);
}
