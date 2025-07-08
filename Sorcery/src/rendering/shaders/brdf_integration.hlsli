#ifndef BRDF_INTEGRATION_HLSLI
#define BRDF_INTEGRATION_HLSLI

#include "brdf.hlsli"
#include "fullscreen_tri.hlsli"
#include "sequences.hlsli"


static uint const kSampleCount = 1024u;


float2 IntegrateBRDF(float const NdotV, float const roughness) {
  float3 V;
  V.x = sqrt(1.0 - NdotV * NdotV);
  V.y = 0.0;
  V.z = NdotV;

  float A = 0.0;
  float B = 0.0;

  float3 const N = float3(0.0, 0.0, 1.0);

  for (uint i = 0u; i < kSampleCount; ++i) {
    float2 const Xi = Hammersley(i, kSampleCount);
    float3 const H = ImportanceSampleGGX(Xi, N, roughness);
    // Very important to keep the precise qualifier here
    // Otherwise optimizations will lead to incorrect results
    precise float3 const L = normalize(2.0 * dot(V, H) * H - V);

    float const NdotL = max(L.z, 0.0);
    float const NdotH = max(H.z, 0.0);
    float const VdotH = max(dot(V, H), 0.0);

    if (NdotL > 0.0) {
      float const G = GeometrySmithIBL(N, V, L, roughness);
      float const G_Vis = (G * VdotH) / (NdotH * NdotV);
      float const Fc = pow(1.0 - VdotH, 5.0);

      A += (1.0 - Fc) * G_Vis;
      B += Fc * G_Vis;
    }
  }

  A /= float(kSampleCount);
  B /= float(kSampleCount);

  return float2(A, B);
}


float2 PsMain(PsIn const ps_in) : SV_Target {
  return IntegrateBRDF(ps_in.uv.x, ps_in.uv.y);
}


#endif
