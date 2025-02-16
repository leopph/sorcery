#ifndef SHADOW_SAMPLING_HLSLI
#define SHADOW_SAMPLING_HLSLI


void GetTent3Weights(const float2 kernelOffset, out float4 weightsX, out float4 weightsY) {
  const float2 a = 0.5 - kernelOffset;
  const float2 b = 0.5 + kernelOffset;
  const float2 c = max(0, -kernelOffset);
  const float2 d = max(0, kernelOffset);
  const float2 w1 = a * a * 0.5;
  const float2 w2 = (1 + a) * (1 + a) * 0.5 - w1 - c * c;
  const float2 w4 = b * b * 0.5;
  const float2 w3 = (1 + b) * (1 + b) * 0.5 - w4 - d * d;
  weightsX = float4(w1.x, w2.x, w3.x, w4.x);
  weightsY = float4(w1.y, w2.y, w3.y, w4.y);
}


void GetTent5Weights(const float kernelOffset, out float4 weightsA, out float2 weightsB) {
  const float a = 0.5 - kernelOffset;
  const float b = 0.5 + kernelOffset;
  const float c = max(0, -kernelOffset);
  const float d = max(0, kernelOffset);
  const float w1 = a * a * 0.5;
  const float w2 = (2 * a + 1) * 0.5;
  const float w3 = (2 + a) * (2 + a) * 0.5 - w1 - w2 - c * c;

  const float w6 = b * b * 0.5;
  const float w5 = (2 * b + 1) * 0.5;
  const float w4 = (2 + b) * (2 + b) * 0.5 - w5 - w6 - d * d;

  weightsA = float4(w1, w2, w3, w4);
  weightsB = float2(w5, w6);
}


float2 GetGroupTapUV(const float2 texelSize, const float2 groupCenterCoord, const float2 weightsX,
                     const float2 weightsY) {
  const float offsetX = weightsX.y / (weightsX.x + weightsX.y);
  const float offsetY = weightsY.y / (weightsY.x + weightsY.y);
  const float2 coord = groupCenterCoord - 0.5 + float2(offsetX, offsetY);
  return coord * texelSize;
}


float4 GetTent3GroupWeights(const float4 weightsX, const float4 weightsY) {
  float4 tapWeights;
  tapWeights.x = dot(weightsX.xyxy, weightsY.xxyy);
  tapWeights.y = dot(weightsX.zwzw, weightsY.xxyy);
  tapWeights.z = dot(weightsX.xyxy, weightsY.zzww);
  tapWeights.w = dot(weightsX.zwzw, weightsY.zzww);
  return tapWeights / dot(tapWeights, 1);
}


void GetTent5GroupWeights(const float4 weightsXA, const float2 weightsXB, const float4 weightsYA,
                          const float2 weightsYB, out float3 groupWeightsA, out float3 groupWeightsB,
                          out float3 groupWeightsC) {
  groupWeightsA.x = dot(weightsXA.xyxy, weightsYA.xxyy);
  groupWeightsA.y = dot(weightsXA.zwzw, weightsYA.xxyy);
  groupWeightsA.z = dot(weightsXB.xyxy, weightsYA.xxyy);

  groupWeightsB.x = dot(weightsXA.xyxy, weightsYA.zzww);
  groupWeightsB.y = dot(weightsXA.zwzw, weightsYA.zzww);
  groupWeightsB.z = dot(weightsXB.xyxy, weightsYA.zzww);

  groupWeightsC.x = dot(weightsXA.xyxy, weightsYB.xxyy);
  groupWeightsC.y = dot(weightsXA.zwzw, weightsYB.xxyy);
  groupWeightsC.z = dot(weightsXB.xyxy, weightsYB.xxyy);
  const float w = dot(groupWeightsA, 1) + dot(groupWeightsB, 1) + dot(groupWeightsC, 1);
  const float iw = rcp(w);
  groupWeightsA *= iw;
  groupWeightsB *= iw;
  groupWeightsC *= iw;
}


float2 GetShadowMapSize(const Texture2D<float> shadowMap) {
  float2 shadowMapSize;
  shadowMap.GetDimensions(shadowMapSize.x, shadowMapSize.y);
  return shadowMapSize;
}


float3 GetShadowMapArraySize(const Texture2DArray<float> shadowMapArray) {
  float3 shadowMapArraySize;
  shadowMapArray.GetDimensions(shadowMapArraySize.x, shadowMapArraySize.y, shadowMapArraySize.z);
  return shadowMapArraySize;
}


float2 GetShadowMapTexelSize(const float2 shadowMapSize) {
  return 1.0 / shadowMapSize;
}


float3 GetShadowMapArrayTexelSize(const float3 shadowMapSize) {
  return 1.0 / shadowMapSize;
}


float2 GetShadowMapTexelSize(const Texture2D<float> shadowMap) {
  return GetShadowMapTexelSize(GetShadowMapSize(shadowMap));
}


float3 GetShadowMapArrayTexelSize(const Texture2DArray<float> shadowMapArray) {
  return GetShadowMapArrayTexelSize(GetShadowMapArraySize(shadowMapArray));
}


void GetPCF3x3Tent4TapParams(const float2 uv, const float2 shadowMapSize, out float2 uv0, out float2 uv1,
                             out float2 uv2, out float2 uv3, out float4 weights) {
  const float2 texelSize = GetShadowMapTexelSize(shadowMapSize);

  const float2 texelCoord = shadowMapSize * uv;
  const float2 texelOriginal = round(texelCoord);
  const float2 kernelOffset = texelCoord - texelOriginal;

  float4 weightsX, weightsY;
  GetTent3Weights(kernelOffset, weightsX, weightsY);

  uv0 = GetGroupTapUV(texelSize, texelOriginal + float2(-1, -1), weightsX.xy, weightsY.xy);
  uv1 = GetGroupTapUV(texelSize, texelOriginal + float2(1, -1), weightsX.zw, weightsY.xy);
  uv2 = GetGroupTapUV(texelSize, texelOriginal + float2(-1, 1), weightsX.xy, weightsY.zw);
  uv3 = GetGroupTapUV(texelSize, texelOriginal + float2(1, 1), weightsX.zw, weightsY.zw);

  weights = GetTent3GroupWeights(weightsX, weightsY);
}


void GetPCF5x5Tent9TapParams(const float2 uv, const float2 shadowMapSize, out float2 uv0, out float2 uv1,
                             out float2 uv2, out float2 uv3, out float2 uv4, out float2 uv5, out float2 uv6,
                             out float2 uv7, out float2 uv8, out float3 groupWeightsA, out float3 groupWeightsB,
                             out float3 groupWeightsC) {
  const float2 texelSize = GetShadowMapTexelSize(shadowMapSize);

  const float2 texelCoord = shadowMapSize * uv;
  const float2 texelOriginal = round(texelCoord);
  const float2 kernelOffset = texelCoord - texelOriginal;

  float4 weightsXA, weightsYA;
  float2 weightsXB, weightsYB;
  GetTent5Weights(kernelOffset.x, weightsXA, weightsXB);
  GetTent5Weights(kernelOffset.y, weightsYA, weightsYB);

  uv0 = GetGroupTapUV(texelSize, texelOriginal + float2(-2, -2), weightsXA.xy, weightsYA.xy);
  uv1 = GetGroupTapUV(texelSize, texelOriginal + float2(0, -2), weightsXA.zw, weightsYA.xy);
  uv2 = GetGroupTapUV(texelSize, texelOriginal + float2(2, -2), weightsXB.xy, weightsYA.xy);
  uv3 = GetGroupTapUV(texelSize, texelOriginal + float2(-2, 0), weightsXA.xy, weightsYA.zw);
  uv4 = GetGroupTapUV(texelSize, texelOriginal + float2(0, 0), weightsXA.zw, weightsYA.zw);
  uv5 = GetGroupTapUV(texelSize, texelOriginal + float2(2, 0), weightsXB.xy, weightsYA.zw);
  uv6 = GetGroupTapUV(texelSize, texelOriginal + float2(-2, 2), weightsXA.xy, weightsYB.xy);
  uv7 = GetGroupTapUV(texelSize, texelOriginal + float2(0, 2), weightsXA.zw, weightsYB.xy);
  uv8 = GetGroupTapUV(texelSize, texelOriginal + float2(2, 2), weightsXB.xy, weightsYB.xy);

  GetTent5GroupWeights(weightsXA, weightsXB, weightsYA, weightsYB, groupWeightsA, groupWeightsB, groupWeightsC);
}


float SampleShadowMapNoFilter(const Texture2D<float> shadowMap, uniform const SamplerComparisonState cmp_point_sampler,
                              const float2 uv, const float depth) {
  return shadowMap.SampleCmpLevelZero(cmp_point_sampler, uv, depth);
}


float SampleShadowMapArrayNoFilter(const Texture2DArray<float> shadowMapArray,
                                   uniform const SamplerComparisonState cmp_point_sampler, const float2 uv,
                                   const uint arraySlice, const float cmpDepth) {
  return shadowMapArray.SampleCmpLevelZero(cmp_point_sampler, float3(uv, arraySlice), cmpDepth);
}


float SampleShadowMapHardwarePCF(const Texture2D<float> shadowMap, uniform const SamplerComparisonState cmp_pcf_samp,
                                 const float2 uv, const float depth) {
  return shadowMap.SampleCmpLevelZero(cmp_pcf_samp, uv, depth);
}


float SampleShadowMapArrayHardwarePCF(const Texture2DArray<float> shadowMapArray,
                                      uniform const SamplerComparisonState cmp_pcf_samp, const float2 uv,
                                      const uint arraySlice, const float cmpDepth) {
  return shadowMapArray.SampleCmpLevelZero(cmp_pcf_samp, float3(uv, arraySlice), cmpDepth);
}


float SampleShadowMapPCF3x34TapFast(const Texture2D<float> shadowMap, uniform const SamplerComparisonState cmp_pcf_samp,
                                    const float2 uv, const float depth) {
  const float2 texelSize = GetShadowMapTexelSize(shadowMap);
  const float2 uvOffset = texelSize * 0.5;

  float4 result;
  result.x = SampleShadowMapHardwarePCF(shadowMap, cmp_pcf_samp, float2(uv.x + uvOffset.x, uv.y + uvOffset.y), depth);
  result.y = SampleShadowMapHardwarePCF(shadowMap, cmp_pcf_samp, float2(uv.x - uvOffset.x, uv.y + uvOffset.y), depth);
  result.z = SampleShadowMapHardwarePCF(shadowMap, cmp_pcf_samp, float2(uv.x - uvOffset.x, uv.y - uvOffset.y), depth);
  result.w = SampleShadowMapHardwarePCF(shadowMap, cmp_pcf_samp, float2(uv.x + uvOffset.x, uv.y - uvOffset.y), depth);

  return dot(result, 0.25);
}


float SampleShadowMapArrayPCF3x34TapFast(const Texture2DArray<float> shadowMapArray,
                                         uniform const SamplerComparisonState cmp_pcf_samp, const float2 uv,
                                         const uint arraySlice, const float depth) {
  const float2 texelSize = GetShadowMapArrayTexelSize(shadowMapArray).xy;
  const float2 uvOffset = texelSize * 0.5;

  float4 result;
  result.x = SampleShadowMapArrayHardwarePCF(shadowMapArray, cmp_pcf_samp, float2(uv.x + uvOffset.x, uv.y + uvOffset.y),
    arraySlice, depth);
  result.y = SampleShadowMapArrayHardwarePCF(shadowMapArray, cmp_pcf_samp, float2(uv.x - uvOffset.x, uv.y + uvOffset.y),
    arraySlice, depth);
  result.z = SampleShadowMapArrayHardwarePCF(shadowMapArray, cmp_pcf_samp, float2(uv.x - uvOffset.x, uv.y - uvOffset.y),
    arraySlice, depth);
  result.w = SampleShadowMapArrayHardwarePCF(shadowMapArray, cmp_pcf_samp, float2(uv.x + uvOffset.x, uv.y - uvOffset.y),
    arraySlice, depth);

  return dot(result, 0.25);
}


float SampleShadowMapPCF3x3Tent4Tap(const Texture2D<float> shadowMap, uniform const SamplerComparisonState cmp_pcf_samp,
                                    const float2 uv, const float depth) {
  float2 uv0, uv1, uv2, uv3;
  float4 weights;
  GetPCF3x3Tent4TapParams(uv, GetShadowMapSize(shadowMap), uv0, uv1, uv2, uv3, weights);

  float4 tap4;
  tap4.x = SampleShadowMapHardwarePCF(shadowMap, cmp_pcf_samp, uv0, depth);
  tap4.y = SampleShadowMapHardwarePCF(shadowMap, cmp_pcf_samp, uv1, depth);
  tap4.z = SampleShadowMapHardwarePCF(shadowMap, cmp_pcf_samp, uv2, depth);
  tap4.w = SampleShadowMapHardwarePCF(shadowMap, cmp_pcf_samp, uv3, depth);

  return dot(tap4, weights);
}


float SampleShadowMapArrayPCF3x3Tent4Tap(const Texture2DArray<float> shadowMapArray,
                                         uniform const SamplerComparisonState cmp_pcf_samp, const float2 uv,
                                         const uint arraySlice, const float depth) {
  float2 uv0, uv1, uv2, uv3;
  float4 weights;
  GetPCF3x3Tent4TapParams(uv, GetShadowMapArraySize(shadowMapArray).xy, uv0, uv1, uv2, uv3, weights);

  float4 tap4;
  tap4.x = SampleShadowMapArrayHardwarePCF(shadowMapArray, cmp_pcf_samp, uv0, arraySlice, depth);
  tap4.y = SampleShadowMapArrayHardwarePCF(shadowMapArray, cmp_pcf_samp, uv1, arraySlice, depth);
  tap4.z = SampleShadowMapArrayHardwarePCF(shadowMapArray, cmp_pcf_samp, uv2, arraySlice, depth);
  tap4.w = SampleShadowMapArrayHardwarePCF(shadowMapArray, cmp_pcf_samp, uv3, arraySlice, depth);

  return dot(tap4, weights);
}


float SampleShadowMapPCF5x5Tent9Tap(const Texture2D<float> shadowMap, uniform const SamplerComparisonState cmp_pcf_samp,
                                    const float2 uv, const float depth) {
  float2 uv0, uv1, uv2, uv3, uv4, uv5, uv6, uv7, uv8;
  float3 groupWeightsA, groupWeightsB, groupWeightsC;
  GetPCF5x5Tent9TapParams(uv, GetShadowMapSize(shadowMap), uv0, uv1, uv2, uv3, uv4, uv5, uv6, uv7, uv8, groupWeightsA,
    groupWeightsB, groupWeightsC);

  float3 tapA, tapB, tapC;

  tapA.x = SampleShadowMapHardwarePCF(shadowMap, cmp_pcf_samp, uv0, depth);
  tapA.y = SampleShadowMapHardwarePCF(shadowMap, cmp_pcf_samp, uv1, depth);
  tapA.z = SampleShadowMapHardwarePCF(shadowMap, cmp_pcf_samp, uv2, depth);

  tapB.x = SampleShadowMapHardwarePCF(shadowMap, cmp_pcf_samp, uv3, depth);
  tapB.y = SampleShadowMapHardwarePCF(shadowMap, cmp_pcf_samp, uv4, depth);
  tapB.z = SampleShadowMapHardwarePCF(shadowMap, cmp_pcf_samp, uv5, depth);

  tapC.x = SampleShadowMapHardwarePCF(shadowMap, cmp_pcf_samp, uv6, depth);
  tapC.y = SampleShadowMapHardwarePCF(shadowMap, cmp_pcf_samp, uv7, depth);
  tapC.z = SampleShadowMapHardwarePCF(shadowMap, cmp_pcf_samp, uv8, depth);

  return dot(tapA, groupWeightsA) + dot(tapB, groupWeightsB) + dot(tapC, groupWeightsC);
}


float SampleShadowMapArrayPCF5x5Tent9Tap(const Texture2DArray<float> shadowMapArray,
                                         uniform const SamplerComparisonState cmp_pcf_samp, const float2 uv,
                                         const uint arraySlice, const float depth) {
  float2 uv0, uv1, uv2, uv3, uv4, uv5, uv6, uv7, uv8;
  float3 groupWeightsA, groupWeightsB, groupWeightsC;
  GetPCF5x5Tent9TapParams(uv, GetShadowMapArraySize(shadowMapArray).xy, uv0, uv1, uv2, uv3, uv4, uv5, uv6, uv7, uv8,
    groupWeightsA, groupWeightsB, groupWeightsC);

  float3 tapA, tapB, tapC;

  tapA.y = SampleShadowMapArrayHardwarePCF(shadowMapArray, cmp_pcf_samp, uv1, arraySlice, depth);
  tapA.x = SampleShadowMapArrayHardwarePCF(shadowMapArray, cmp_pcf_samp, uv0, arraySlice, depth);
  tapA.z = SampleShadowMapArrayHardwarePCF(shadowMapArray, cmp_pcf_samp, uv2, arraySlice, depth);

  tapB.x = SampleShadowMapArrayHardwarePCF(shadowMapArray, cmp_pcf_samp, uv3, arraySlice, depth);
  tapB.y = SampleShadowMapArrayHardwarePCF(shadowMapArray, cmp_pcf_samp, uv4, arraySlice, depth);
  tapB.z = SampleShadowMapArrayHardwarePCF(shadowMapArray, cmp_pcf_samp, uv5, arraySlice, depth);

  tapC.x = SampleShadowMapArrayHardwarePCF(shadowMapArray, cmp_pcf_samp, uv6, arraySlice, depth);
  tapC.y = SampleShadowMapArrayHardwarePCF(shadowMapArray, cmp_pcf_samp, uv7, arraySlice, depth);
  tapC.z = SampleShadowMapArrayHardwarePCF(shadowMapArray, cmp_pcf_samp, uv8, arraySlice, depth);

  return dot(tapA, groupWeightsA) + dot(tapB, groupWeightsB) + dot(tapC, groupWeightsC);
}

#endif
