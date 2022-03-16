struct VSOutput
{
    float4 Position: SV_POSITION;
    float3 Color: COLOR0;
    float2 Uv: UV0;
};

sampler LinearSampler : register(s0);

Texture2D Texture : register(t0);

float4 Main(VSOutput input): SV_TARGET
{
    float4 texel = Texture.Sample(LinearSampler, input.Uv);
    //return float4(input.Color, 1.0f) + 0.5 * texel;
    return texel;
}
