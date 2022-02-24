struct vs_in {
    float3 verts: POSITION;
    float3 color: COLOR;
};

struct vs_out {
    float4 verts: SV_POSITION;
    float3 color: COLOR0;
};

vs_out main(vs_in input) {
    vs_out output = (vs_out)0;
    output.color = input.color;
    output.verts = float4(input.verts, 1.0);
    return output;
}
