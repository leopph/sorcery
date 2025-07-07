#ifndef BRDF_INTEGRATION_HLSLI
#define BRDF_INTEGRATION_HLSLI

#include "brdf.hlsli"
#include "fullscreen_tri.hlsli"
#include "sequences.hlsli"


float2 IntegrateBRDF(float const NdotV, float const roughness) {
  float3 V;
  V.x = sqrt(1.0 - NdotV * NdotV);
  V.y = 0.0;
  V.z = NdotV;

  float A = 0.0;
  float B = 0.0;

  float3 N = float3(0.0, 0.0, 1.0);

  uint const sample_count = 1024u;

  for (uint i = 0u; i < sample_count; ++i) {
    float2 Xi = Hammersley(i, sample_count);
    float3 H = ImportanceSampleGGX(Xi, N, roughness);
    // Very important to keep the precise qualifier here
    // Otherwise optimizations will lead to incorrect results
    precise float3 L = normalize(2.0 * dot(V, H) * H - V);

    float NdotL = max(L.z, 0.0);
    float NdotH = max(H.z, 0.0);
    float VdotH = max(dot(V, H), 0.0);

    if (NdotL > 0.0) {
      float G = GeometrySmithIBL(N, V, L, roughness);
      float G_Vis = (G * VdotH) / (NdotH * NdotV);
      float Fc = pow(1.0 - VdotH, 5.0);

      A += (1.0 - Fc) * G_Vis;
      B += Fc * G_Vis;
    }
  }

  A /= float(sample_count);
  B /= float(sample_count);

  return float2(A, B);
}


float2 PsMain(PsIn const ps_in) : SV_Target {
  return IntegrateBRDF(ps_in.uv.x, ps_in.uv.y);
}


#endif
