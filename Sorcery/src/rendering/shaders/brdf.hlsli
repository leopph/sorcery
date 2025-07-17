#ifndef BRDF_HLSLI
#define BRDF_HLSLI
#include "common.hlsli"


static float const PI = 3.14159265359;


float DistributionTrowbridgeReitz(float const n_dot_h, float const roughness) {
  float const a = roughness * roughness;
  float const a2 = a * a;
  float const n_dot_h2 = n_dot_h * n_dot_h;
  float denom = n_dot_h2 * (a2 - 1.0) + 1.0;
  denom = PI * denom * denom;
  return a2 / denom;
}


// Use for direct lighting
float GeometrySchlickTrowbridgeReitzDirect(float const NdotV, float const roughness) {
  float const r = roughness + 1.0;
  float const k = pow(r, 2) / 8.0;

  float const num = NdotV;
  float const denom = NdotV * (1.0 - k) + k;

  return num / denom;
}


// Use for IBL
float GeometrySchlickTrowbridgeReitzIBL(float const NdotV, float const roughness) {
  float const a = roughness;
  float const k = (a * a) / 2.0;

  float const nom = NdotV;
  float const denom = NdotV * (1.0 - k) + k;

  return nom / denom;
}


// Use for direct lighting
float GeomertySmithDirect(float3 const N, float3 const V, float3 const L, float const roughness) {
  float const NdotV = max(dot(N, V), 0.0);
  float const NdotL = max(dot(N, L), 0.0);
  float const ggx2 = GeometrySchlickTrowbridgeReitzDirect(NdotV, roughness);
  float const ggx1 = GeometrySchlickTrowbridgeReitzDirect(NdotL, roughness);

  return ggx1 * ggx2;
}


// Use for IBL
float GeometrySmithIBL(float3 const N, float3 const V, float3 const L, float const roughness) {
  float const NdotV = max(dot(N, V), 0.0);
  float const NdotL = max(dot(N, L), 0.0);
  float const ggx2 = GeometrySchlickTrowbridgeReitzIBL(NdotV, roughness);
  float const ggx1 = GeometrySchlickTrowbridgeReitzIBL(NdotL, roughness);

  return ggx1 * ggx2;
}


float3 FresnelSchlick(float const cosTheta, float3 const F0) {
  return F0 + (1.0 - F0) * pow(saturate(1.0 - cosTheta), 5.0);
}


float3 FresnelSchlickRoughness(float const cosTheta, float3 const F0, float roughness) {
  return F0 + (max((float3)(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
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

  // cook-torrance brdf
  float const NDF = DistributionTrowbridgeReitz(n_dot_h, roughness);
  float const G = GeomertySmithDirect(N, V, L, roughness);
  float3 const F = FresnelSchlick(max(dot(H, V), 0.0), F0);

  float3 const numerator = NDF * G * F;
  float const denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
  float3 const specular = numerator / denominator;

  float3 const kS = F;
  float3 kD = 1.0 - kS;
  kD *= 1.0 - metallic;

  float const NdotL = max(dot(N, L), 0.0);

  // outgoing radiance
  return (kD * albedo / PI + specular) * radiance * NdotL;
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
