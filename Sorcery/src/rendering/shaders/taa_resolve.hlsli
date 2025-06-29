#ifndef TAA_RESOLVE_HLSLI
#define TAA_RESOLVE_HLSLI

#include "CatmullRom.hlsli"
#include "common.hlsli"
#include "fullscreen_tri.hlsli"
#include "shader_interop.h"

DECLARE_PARAMS(TaaResolveDrawParams);


// https://github.com/TheRealMJP/MSAAFilter/tree/master/MSAAFilter
float FilterCubic1D(float x, float const B, float const C) {
  // Rescale from [-1, 1] range to [-2, 2]
  x = abs(x) * 2.0f;

  float y = 0.0f;
  float x2 = x * x;
  float x3 = x * x * x;
  if (x < 1) {
    y = (12 - 9 * B - 6 * C) * x3 + (-18 + 12 * B + 6 * C) * x2 + (6 - 2 * B);
  } else if (x <= 2) {
    y = (-B - 6 * C) * x3 + (6 * B + 30 * C) * x2 + (-12 * B - 48 * C) * x + (8 * B + 24 * C);
  }

  return y / 6.0f;
}


float FilterMitchell1D(float const x) {
  return FilterCubic1D(x, 1 / 3.0f, 1 / 3.0f);
}


static float const FLT_EPS = 0.00000001f;

// https://github.com/playdeadgames/temporal/blob/master/Assets/Shaders/TemporalReprojection.shader
float4 clip_aabb(float3 aabb_min, float3 aabb_max, float4 p, float4 q) {
#if USE_OPTIMIZATIONS
		// note: only clips towards aabb center (but fast!)
		float3 p_clip = 0.5 * (aabb_max + aabb_min);
		float3 e_clip = 0.5 * (aabb_max - aabb_min) + FLT_EPS;

		float4 v_clip = q - float4(p_clip, p.w);
		float3 v_unit = v_clip.xyz / e_clip;
		float3 a_unit = abs(v_unit);
		float ma_unit = max(a_unit.x, max(a_unit.y, a_unit.z));

		if (ma_unit > 1.0)
			return float4(p_clip, p.w) + v_clip / ma_unit;
		else
			return q;// point inside aabb
#else
  float4 r = q - p;
  float3 rmax = aabb_max - p.xyz;
  float3 rmin = aabb_min - p.xyz;

  float const eps = FLT_EPS;

  if (r.x > rmax.x + eps) {
    r *= (rmax.x / r.x);
  }
  if (r.y > rmax.y + eps) {
    r *= (rmax.y / r.y);
  }
  if (r.z > rmax.z + eps) {
    r *= (rmax.z / r.z);
  }

  if (r.x < rmin.x - eps) {
    r *= (rmin.x / r.x);
  }
  if (r.y < rmin.y - eps) {
    r *= (rmin.y / r.y);
  }
  if (r.z < rmin.z - eps) {
    r *= (rmin.z / r.z);
  }

  return p + r;
#endif
}


float Luminance(float3 const color) {
  return dot(color, float3(0.2127, 0.7152, 0.0722));
}


// https://alextardif.com/TAA.html
float4 PsMain(PsIn const ps_in) : SV_Target {
  Texture2D const color_tex = ResourceDescriptorHeap[g_params.color_tex_idx];
  Texture2D const accum_tex = ResourceDescriptorHeap[g_params.accum_tex_idx];
  Texture2D<float> const depth_tex = ResourceDescriptorHeap[g_params.depth_tex_idx];
  Texture2D<float2> const velocity_tex = ResourceDescriptorHeap[g_params.velocity_tex_idx];

  SamplerState const linear_samp = SamplerDescriptorHeap[g_params.linear_samp_idx];

  uint2 color_tex_size;
  color_tex.GetDimensions(color_tex_size.x, color_tex_size.y);

  uint2 accum_tex_size;
  accum_tex.GetDimensions(accum_tex_size.x, accum_tex_size.y);

  float3 sourceSampleTotal = float3(0, 0, 0);
  float sourceSampleWeight = 0.0;
  float3 neighborhoodMin = 10000;
  float3 neighborhoodMax = -10000;
  float3 m1 = float3(0, 0, 0);
  float3 m2 = float3(0, 0, 0);
  float closestDepth = 0.0;
  int2 closestDepthPixelPosition = int2(0, 0);

  for (int x = -1; x <= 1; x++) {
    for (int y = -1; y <= 1; y++) {
      int2 pixelPosition = ps_in.pos_cs.xy + int2(x, y);
      pixelPosition = clamp(pixelPosition, 0, color_tex_size - 1);

      float3 neighbor = max(0, color_tex[pixelPosition].rgb);
      float subSampleDistance = length(float2(x, y));
      float subSampleWeight = FilterMitchell1D(subSampleDistance);

      sourceSampleTotal += neighbor * subSampleWeight;
      sourceSampleWeight += subSampleWeight;

      neighborhoodMin = min(neighborhoodMin, neighbor);
      neighborhoodMax = max(neighborhoodMax, neighbor);

      m1 += neighbor;
      m2 += neighbor * neighbor;

      float currentDepth = depth_tex[pixelPosition].r;
      if (currentDepth > closestDepth) {
        closestDepth = currentDepth;
        closestDepthPixelPosition = pixelPosition;
      }
    }
  }

  float2 motionVector = velocity_tex[closestDepthPixelPosition].xy * float2(0.5, -0.5);
  float2 historyTexCoord = ps_in.uv.xy - motionVector;
  float3 sourceSample = sourceSampleTotal / sourceSampleWeight;

  if (any(historyTexCoord != saturate(historyTexCoord))) {
    return float4(sourceSample, 1);
  }

  float3 historySample = SampleTextureCatmullRom(accum_tex, linear_samp, historyTexCoord, float2(accum_tex_size.xy)).
    rgb;

  float oneDividedBySampleCount = 1.0 / 9.0;
  float gamma = 1.0;
  float3 mu = m1 * oneDividedBySampleCount;
  float3 sigma = sqrt(abs((m2 * oneDividedBySampleCount) - (mu * mu)));
  float3 minc = mu - gamma * sigma;
  float3 maxc = mu + gamma * sigma;

  //TODO historySample = clip_aabb(minc, maxc, clamp(historySample, neighborhoodMin, neighborhoodMax));
  historySample = clamp(historySample, neighborhoodMin, neighborhoodMax);

  float sourceWeight = 0.05;
  float historyWeight = 1.0 - sourceWeight;
  float3 compressedSource = sourceSample * rcp(max(max(sourceSample.r, sourceSample.g), sourceSample.b) + 1.0);
  float3 compressedHistory = historySample * rcp(max(max(historySample.r, historySample.g), historySample.b) + 1.0);
  float luminanceSource = Luminance(compressedSource);
  float luminanceHistory = Luminance(compressedHistory);

  sourceWeight *= 1.0 / (1.0 + luminanceSource);
  historyWeight *= 1.0 / (1.0 + luminanceHistory);

  float3 result = (sourceSample * sourceWeight + historySample * historyWeight) / max(sourceWeight + historyWeight,
                    0.00001);

  return float4(result, 1);
}

#endif
