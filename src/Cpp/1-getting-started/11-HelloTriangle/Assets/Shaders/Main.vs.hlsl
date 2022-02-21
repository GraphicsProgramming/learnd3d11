struct vs_in {
    float3 local: POSITION;
};

struct vs_out {
    float4 clip: SV_POSITION;
};

vs_out main(vs_in input) {
  vs_out output = (vs_out)0;
  output.clip = float4(input.local, 1.0);
  return output;
}

