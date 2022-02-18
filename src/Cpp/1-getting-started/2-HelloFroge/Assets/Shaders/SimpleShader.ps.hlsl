struct PixelInput
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
    float2 Uv : TEXCOORD;
};

sampler LinearSampler : register(s0);

Texture2D Texture : register(t0);

float4 Main(PixelInput input) : SV_TARGET
{
	float3 texel = Texture.Sample(LinearSampler, input.Uv).rgb;
	return input.Color * float4(texel, 1.0f);
}