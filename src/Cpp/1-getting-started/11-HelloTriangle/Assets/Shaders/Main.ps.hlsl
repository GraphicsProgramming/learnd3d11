struct vs_out {
    float4 clip: SV_POSITION;
};

float4 main(vs_out input): SV_TARGET {
    return float4(1.0, 1.0, 1.0, 1.0); 
}

