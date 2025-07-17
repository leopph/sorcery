#ifndef BRDF_INTEGRATION_HLSLI
#define BRDF_INTEGRATION_HLSLI

#include "brdf.hlsli"
#include "fullscreen_tri.hlsli"
#include "sequences.hlsli"


static uint const kSampleCount = 1024u;


float2 IntegrateBRDF(float const n_dot_v, float const roughness) {
  float3 V;
  V.x = sqrt(1.0 - n_dot_v * n_dot_v);
  V.y = 0.0;
  V.z = n_dot_v;

  float A = 0.0;
  float B = 0.0;

  float3 const N = float3(0.0, 0.0, 1.0);

  for (uint i = 0u; i < kSampleCount; ++i) {
    float2 const Xi = Hammersley(i, kSampleCount);
    float3 const H = ImportanceSampleGGX(Xi, N, roughness);

    // Very important to keep the precise qualifier here
    // Otherwise optimizations will lead to incorrect results
    precise float3 const L = normalize(2.0 * dot(V, H) * H - V);

    float const n_dot_l = saturate(L.z);
    float const n_dot_h = saturate(H.z);
    float const v_dot_h = saturate(dot(V, H));

    if (n_dot_l > 0.0) {
      float const G = GeometrySmithIbl(n_dot_v, n_dot_l, roughness);

      float const G_Vis = (G * v_dot_h) / (n_dot_h * n_dot_v);
      float const Fc = pow(1.0 - v_dot_h, 5.0);
      A += (1.0 - Fc) * G_Vis;
      B += Fc * G_Vis;
    }
  }

  return float2(A, B) / kSampleCount;
}


float2 PsMain(PsIn const ps_in) : SV_Target {
  return IntegrateBRDF(ps_in.uv.x, ps_in.uv.y);
}


#endif
