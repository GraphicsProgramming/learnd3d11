struct VSOutput
{
    float4 position: SV_Position;
    float3 color: COLOR0;
};

float4 Main(VSOutput input): SV_Target
{
    return float4(input.color, 1.0); 
}
