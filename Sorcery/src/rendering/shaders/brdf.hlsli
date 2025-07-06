#ifndef BRDF_HLSLI
#define BRDF_HLSLI


static float const PI = 3.14159265359;


float DistributionTrowbridgeReitz(float3 const N, float3 const H, float const roughness) {
  float const a = pow(roughness, 2);
  float const a2 = pow(a, 2);
  float const NdotH = max(dot(N, H), 0.0);
  float const NdotH2 = pow(NdotH, 2);

  float const num = a2;
  float denom = (NdotH2 * (a2 - 1.0) + 1.0);
  denom = PI * pow(denom, 2);

  return num / denom;
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

  // cook-torrance brdf
  float const NDF = DistributionTrowbridgeReitz(N, H, roughness);
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

#endif
