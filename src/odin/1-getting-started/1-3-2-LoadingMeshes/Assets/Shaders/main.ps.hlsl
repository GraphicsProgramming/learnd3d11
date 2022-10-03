struct VSOutput
{
    float4 position: SV_Position;
    float3 color: COLOR0;
    float2 uv: TEXCOORD0;
};

sampler LinearSampler: register(s0);

Texture2D Texture: register(t0);

float4 Main(VSOutput input): SV_Target
{
    input.uv.y = -input.uv.y;
    float4 texel = Texture.Sample(LinearSampler, input.uv);
    return float4(input.color, 0.5f) * texel;
}