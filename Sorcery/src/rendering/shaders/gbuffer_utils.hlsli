#ifndef GBUFFER_UTILS_HLSLI
#define GBUFFER_UTILS_HLSLI

// https://aras-p.info/texts/CompactNormalStorage.html#method04spheremap

float2 EncodeNormal(float3 n) {
  float2 scale = 1.7777;
  float2 enc = n.xy / (n.z + 1);
  enc /= scale;
  enc = enc * 0.5 + 0.5;
  return enc;
}


float3 DecodeNormal(float2 enc) {
  float scale = 1.7777;
  float3 nn = float3(enc.xy, 0) * float3(2 * scale, 2 * scale, 0) + float3(-scale, -scale, 1);
  float g = 2.0 / dot(nn.xyz, nn.xyz);
  float3 n;
  n.xy = g * nn.xy;
  n.z = g - 1;
  return n;
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
