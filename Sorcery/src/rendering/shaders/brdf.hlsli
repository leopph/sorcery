#ifndef BRDF_HLSLI
#define BRDF_HLSLI

#include "common.hlsli"


float DistributionTrowbridgeReitz(float const n_dot_h, float const roughness) {
  float const a = roughness * roughness;
  float const a2 = a * a;
  float const n_dot_h2 = n_dot_h * n_dot_h;
  float denom = n_dot_h2 * (a2 - 1.0) + 1.0;
  denom = kPi * denom * denom;
  return a2 / denom;
}


// Use for direct lighting
float GeometrySchlickTrowbridgeReitzDirect(float const n_dot_v, float const roughness) {
  float const r = roughness + 1.0;
  float const k = r * r / 8.0;
  return n_dot_v / (n_dot_v * (1.0 - k) + k);
}


// Use for IBL
float GeometrySchlickTrowbridgeReitzIbl(float const n_dot_v, float const roughness) {
  float const k = (roughness * roughness) / 2.0;
  return n_dot_v / (n_dot_v * (1.0 - k) + k);
}


// Use for direct lighting
float GeomertySmithDirect(float const n_dot_v, float const n_dot_l, float const roughness) {
  float const ggx2 = GeometrySchlickTrowbridgeReitzDirect(n_dot_v, roughness);
  float const ggx1 = GeometrySchlickTrowbridgeReitzDirect(n_dot_l, roughness);

  return ggx1 * ggx2;
}


// Use for IBL
float GeometrySmithIbl(float const n_dot_v, float const n_dot_l, float const roughness) {
  float const ggx2 = GeometrySchlickTrowbridgeReitzIbl(n_dot_v, roughness);
  float const ggx1 = GeometrySchlickTrowbridgeReitzIbl(n_dot_l, roughness);

  return ggx1 * ggx2;
}


float3 FresnelSchlick(float const v_dot_h, float3 const f0) {
  return f0 + (1.0 - f0) * pow(saturate(1.0 - v_dot_h), 5.0);
}


float3 FresnelSchlickRoughness(float const v_dot_h, float3 const f0, float roughness) {
  return f0 + (max((float3)(1.0 - roughness), f0) - f0) * pow(clamp(1.0 - v_dot_h, 0.0, 1.0), 5.0);
}


float3 CalcF0(float3 const albedo, float const metallic) {
  // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
  // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)
  return lerp((float3)0.04, albedo, metallic);
}


float3 CookTorrance(float3 const N, float3 const V, float3 const L, float3 const albedo, float const metallic,
                    float const roughness, float3 const lightColor, float const lightIntensity,
                    float const lightAtten) {
  float3 const F0 = CalcF0(albedo, metallic);

  // calculate per-light radiance
  float3 const H = normalize(V + L);
  float3 const radiance = lightColor * lightIntensity * lightAtten;

  float const n_dot_h = saturate(dot(N, H));
  float const n_dot_v = saturate(dot(N, V));
  float const n_dot_l = saturate(dot(N, L));
  float const v_dot_h = saturate(dot(V, H));

  // cook-torrance brdf
  float const NDF = DistributionTrowbridgeReitz(n_dot_h, roughness);
  float const G = GeomertySmithDirect(n_dot_v, n_dot_l, roughness);
  float3 const F = FresnelSchlick(v_dot_h, F0);

  float3 const numerator = NDF * G * F;
  float const denominator = 4.0 * n_dot_v * n_dot_l + 0.0001;
  float3 const specular = numerator / denominator;

  float3 const kS = F;
  float3 kD = 1.0 - kS;
  kD *= 1.0 - metallic;

  // outgoing radiance
  return (kD * albedo / kPi + specular) * radiance * n_dot_l;
}


float3 ImportanceSampleGGX(float2 const Xi, float3 const N, float const roughness) {
  float a = roughness * roughness;

  float phi = 2.0 * kPi * Xi.x;
  float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
  float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

  // from spherical coordinates to cartesian coordinates
  float3 H;
  H.x = cos(phi) * sinTheta;
  H.y = sin(phi) * sinTheta;
  H.z = cosTheta;

  // from tangent-space floattor to world-space sample floattor
  float3 up = abs(N.z) < 0.999 ? float3(0.0, 0.0, 1.0) : float3(1.0, 0.0, 0.0);
  float3 tangent = normalize(cross(up, N));
  float3 bitangent = cross(N, tangent);

  float3 samplefloat = tangent * H.x + bitangent * H.y + N * H.z;
  return normalize(samplefloat);
}

#endif
