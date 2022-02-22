struct vs_out {
    float4 verts: SV_POSITION;
    float3 color: COLOR0;
};

float4 main(vs_out input): SV_TARGET {
    return float4(input.color, 1.0);
}
