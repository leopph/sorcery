#ifndef GBUFFER_UTILS_HLSLI
#define GBUFFER_UTILS_HLSLI

// https://www.shadertoy.com/view/cljGD1

float2 OctWrap(float2 v) {
  float2 w = 1.0 - abs(v.yx);
  if (v.x < 0.0) {
    w.x = -w.x;
  }
  if (v.y < 0.0) {
    w.y = -w.y;
  }
  return w;
}


float2 EncodeNormal(float3 n) {
  n /= (abs(n.x) + abs(n.y) + abs(n.z));
  n.xy = n.z > 0.0 ? n.xy : OctWrap(n.xy);
  n.xy = n.xy * 0.5 + 0.5;
  return n.xy;
}


float3 DecodeNormal(float2 f) {
  f = f * 2.0 - 1.0;

  // https://twitter.com/Stubbesaurus/status/937994790553227264
  float3 n = float3(f.x, f.y, 1.0 - abs(f.x) - abs(f.y));
  float t = max(-n.z, 0.0);
  n.x += n.x >= 0.0 ? -t : t;
  n.y += n.y >= 0.0 ? -t : t;
  return normalize(n);
}


float4 PackGBuffer0(float3 const albedo, float const ao) {
  return float4(albedo, ao);
}


void UnpackGBuffer0(float4 const packed, out float3 albedo, out float ao) {
  albedo = packed.rgb;
  ao = packed.a;
}


float2 PackGBuffer1(float3 const normal_ws) {
  return EncodeNormal(normal_ws);
}


void UnpackGBuffer1(float2 const packed, out float3 normal_ws) {
  normal_ws = DecodeNormal(packed.xy);
}


float2 PackGBuffer2(float const roughness, float const metallic) {
  return float2(roughness, metallic);
}


void UnpackGBuffer2(float2 const packed, out float roughness, out float metallic) {
  roughness = packed.x;
  metallic = packed.y;
}

#endif
