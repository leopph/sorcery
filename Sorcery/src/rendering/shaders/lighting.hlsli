#ifndef LIGHTING_HLSLI
#define LIGHTING_HLSLI

#include "brdf.hlsli"
#include "shader_interop.h"
#include "shadow_filtering_modes.h"
#include "shadow_sampling.hlsli"


uint CalculateShadowCascadeIdx(float const pos_vs_z, uniform float4 const cascade_splits) {
  return (uint)dot(cascade_splits < pos_vs_z, 1.0);
}


float3 VisualizeShadowCascades(float const pos_vs_z, uniform float4 const cascade_splits,
                               uniform uint const cascade_count) {
  uint const cascade_idx = CalculateShadowCascadeIdx(pos_vs_z, cascade_splits);

  if (cascade_idx >= cascade_count) {
    return float3(1, 1, 1);
  }

  float3 ret;

  switch (cascade_idx) {
    case 0:
      ret = float3(108, 110, 160);
      break;
    case 1:
      ret = float3(184, 216, 186);
      break;
    case 2:
      ret = float3(252, 221, 188);
      break;
    case 3:
      ret = float3(239, 149, 157);
      break;
    default:
      ret = float3(1, 1, 1); // This should never be reached
      break;
  }

  return pow(ret / 255.0, 2.2);
}


void CalculateShadowSamplingCoordinates(float3 const pos_ws, float3 const normal_ws, float const shadow_map_texel_size,
                                        float const depth_bias, float const normal_bias, float4x4 const view_proj_mtx,
                                        out float2 uv, out float depth) {
  float4 const pos_light_cs = mul(float4(pos_ws + normal_ws * shadow_map_texel_size * normal_bias, 1), view_proj_mtx);
  float3 const pos_light_ndc = pos_light_cs.xyz / pos_light_cs.w;
  uv = pos_light_ndc.xy * float2(0.5, -0.5) + 0.5;

  float const depth_bias_multiplier =
#ifdef REVERSE_Z
    -1.0;
#else
    1.0;
#endif

  depth = pos_light_ndc.z + depth_bias_multiplier * shadow_map_texel_size * -depth_bias;
}


float SampleShadowCascadeFromAtlas(Texture2D<float> const atlas, uniform SamplerComparisonState const shadow_samp,
                                   ShaderLight const light, uint const shadow_map_idx, float3 const pos_ws,
                                   float3 const normal_ws, uniform int const shadow_filtering_mode) {
  uint atlas_size;
  atlas.GetDimensions(atlas_size, atlas_size);
  float const atlas_texel_size = 1.0 / atlas_size;
  float const shadow_map_texel_size = atlas_texel_size / light.shadowAtlasCellSizes[shadow_map_idx];

  float2 uv;
  float depth;
  CalculateShadowSamplingCoordinates(pos_ws, normal_ws, shadow_map_texel_size, light.depthBias, light.normalBias,
    light.shadowViewProjMatrices[shadow_map_idx], uv, depth);

  uv *= light.shadowAtlasCellSizes[shadow_map_idx];
  uv += light.shadowAtlasCellOffsets[shadow_map_idx];

  switch (shadow_filtering_mode) {
    case SHADOW_FILTERING_NONE:
      return SampleShadowMapNoFilter(atlas, shadow_samp, uv, depth);
    case SHADOW_FILTERING_HARDWARE_PCF:
      return SampleShadowMapHardwarePCF(atlas, shadow_samp, uv, depth);
    case SHADOW_FILTERING_PCF_3x3:
      return SampleShadowMapPCF3x34TapFast(atlas, shadow_samp, uv, depth);
    case SHADOW_FILTERING_PCF_TENT_3x3:
      return SampleShadowMapPCF3x3Tent4Tap(atlas, shadow_samp, uv, depth);
    case SHADOW_FILTERING_PCF_TENT_5x5:
      return SampleShadowMapPCF5x5Tent9Tap(atlas, shadow_samp, uv, depth);
    default:
      return 1.0;
  }
}


float SampleShadowCascadeFromArray(Texture2DArray<float> const shadow_map_array,
                                   uniform SamplerComparisonState const shadow_samp, ShaderLight const light,
                                   uint const cascade_idx, float3 const pos_ws, float3 const normal_ws,
                                   uniform int const shadow_filtering_mode) {
  float const shadow_map_texel_size = GetShadowMapArrayTexelSize(shadow_map_array).x;

  float2 uv;
  float depth;
  CalculateShadowSamplingCoordinates(pos_ws, normal_ws, shadow_map_texel_size, light.depthBias, light.normalBias,
    light.shadowViewProjMatrices[cascade_idx], uv, depth);

  switch (shadow_filtering_mode) {
    case SHADOW_FILTERING_NONE:
      return SampleShadowMapArrayNoFilter(shadow_map_array, shadow_samp, uv, cascade_idx,
        depth);
    case SHADOW_FILTERING_HARDWARE_PCF:
      return SampleShadowMapArrayHardwarePCF(shadow_map_array, shadow_samp, uv,
        cascade_idx, depth);
    case SHADOW_FILTERING_PCF_3x3:
      return SampleShadowMapArrayPCF3x34TapFast(shadow_map_array, shadow_samp, uv,
        cascade_idx, depth);
    case SHADOW_FILTERING_PCF_TENT_3x3:
      return SampleShadowMapArrayPCF3x3Tent4Tap(shadow_map_array, shadow_samp, uv,
        cascade_idx, depth);
    case SHADOW_FILTERING_PCF_TENT_5x5:
      return SampleShadowMapArrayPCF5x5Tent9Tap(shadow_map_array, shadow_samp, uv,
        cascade_idx, depth);
    default:
      return 1.0;
  }
}


float CalculateAttenuation(float const distance) {
  return 1 / pow(distance, 2);
}


float3 CalculateDirLight(ShaderLight const light, float3 const pos_ws, float3 const normal_ws,
                         float3 const dir_to_cam_ws, float const pos_vs_z, float3 const albedo, float const metallic,
                         float const roughness, Texture2DArray<float> const shadow_map_arr,
                         uniform SamplerComparisonState const shadow_samp, uniform int const shadow_filtering_mode,
                         uniform float4 const cascade_splits, uniform uint const cascade_count,
                         uniform bool const visualize_cascades) {
  float3 const dir_to_light_ws = -light.direction;

  float3 lighting;

  [branch]
  if (visualize_cascades) {
    lighting = VisualizeShadowCascades(pos_vs_z, cascade_splits, cascade_count);
  } else {
    lighting = CookTorrance(normal_ws, dir_to_cam_ws, dir_to_light_ws, albedo, metallic, roughness, light.color,
      light.intensity, 1);
  }

  [branch]
  if (light.isCastingShadow) {
    uint const cascade_idx = CalculateShadowCascadeIdx(pos_vs_z, cascade_splits);

    [branch]
    if (cascade_idx < cascade_count) {
      lighting *= SampleShadowCascadeFromArray(shadow_map_arr, shadow_samp, light, cascade_idx, pos_ws, normal_ws,
        shadow_filtering_mode);
    }
  }

  return lighting;
}


float3 CalculateSpotLight(ShaderLight const light, float3 const pos_ws, float3 const normal_ws,
                          float3 const dir_to_cam_ws, float3 const albedo, float const metallic, float const roughness,
                          Texture2D<float> const shadow_atlas, uniform SamplerComparisonState const shadow_samp,
                          uniform int const shadow_filtering_mode) {
  float3 dir_to_light_ws = light.position - pos_ws;
  float const dist = length(dir_to_light_ws);
  dir_to_light_ws = normalize(dir_to_light_ws);

  float const range_mul = float(dist <= light.range);
  float const theta_cos = dot(dir_to_light_ws, -light.direction);
  float const eps = light.halfInnerAngleCos - light.halfOuterAngleCos;
  float const intensity = saturate((theta_cos - light.halfOuterAngleCos) / eps);

  float3 lighting = CookTorrance(normal_ws, dir_to_cam_ws, dir_to_light_ws, albedo, metallic, roughness, light.color,
    light.intensity, CalculateAttenuation(dist));
  lighting *= intensity;
  lighting *= range_mul;

  [branch]
  if (light.isCastingShadow) {
    lighting *= SampleShadowCascadeFromAtlas(shadow_atlas, shadow_samp, light, 0, pos_ws, normal_ws,
      shadow_filtering_mode);
  }

  return lighting;
}


float3 CalculatePointLight(ShaderLight const light, float3 const pos_ws, float3 const normal_ws,
                           float3 const dir_to_cam_ws, float3 const albedo, float const metallic, float const roughness,
                           Texture2D<float> const shadow_atlas, uniform SamplerComparisonState const shadow_samp,
                           uniform int const shadow_filtering_mode) {
  float3 dir_to_light_ws = light.position - pos_ws;
  float const dist = length(dir_to_light_ws);
  dir_to_light_ws = normalize(dir_to_light_ws);

  float const range_mul = float(dist <= light.range);

  float3 lighting = CookTorrance(normal_ws, dir_to_cam_ws, dir_to_light_ws, albedo, metallic, roughness, light.color,
    light.intensity, CalculateAttenuation(dist));
  lighting *= range_mul;

  [branch]
  if (light.isCastingShadow) {
    float3 const dir_from_light_ws = pos_ws - light.position;

    uint max_idx = abs(dir_from_light_ws.x) > abs(dir_from_light_ws.y) ? 0 : 1;
    max_idx = abs(dir_from_light_ws[max_idx]) > abs(dir_from_light_ws.z) ? max_idx : 2;
    uint shadow_map_idx = max_idx * 2;

    if (sign(dir_from_light_ws[max_idx]) < 0) {
      shadow_map_idx += 1;
    }

    [branch]
    if (light.sampleShadowMap[shadow_map_idx]) {
      lighting *= SampleShadowCascadeFromAtlas(shadow_atlas, shadow_samp, light, shadow_map_idx, pos_ws, normal_ws,
        shadow_filtering_mode);
    }
  }

  return lighting;
}

#endif
