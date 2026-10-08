#ifndef LIGHTING_HLSLI
#define LIGHTING_HLSLI

#include "brdf.hlsli"
#include "shader_interop.h"
#include "shadow_filtering_modes.h"
#include "shadow_sampling.hlsli"


uint ComputeCascadeIdx(
  float const pos_vs_z,
  float4 const cascade_splits
) {
  return (uint)dot(cascade_splits < pos_vs_z, 1.0);
}


float3 VisualizeShadowCascades(
  float const pos_vs_z,
  float4 const cascade_splits,
  uint const cascade_count
) {
  uint const cascade_idx = ComputeCascadeIdx(pos_vs_z, cascade_splits);

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


float3 EvaluateDirectionalLight(
  ShaderLight const light,
  float3 const normal_ws,
  float3 const dir_to_cam_ws,
  float3 const albedo,
  float const metallic,
  float const roughness
) {
  float3 const dir_to_light_ws = -light.direction;
  return CookTorrance(normal_ws, dir_to_cam_ws, dir_to_light_ws, albedo, metallic, roughness, light.color,
    light.intensity, 1);
}


float CalculateAttenuation(float const distance) {
  return 1 / pow(distance, 2);
}


float3 EvaluateSpotLight(
  ShaderLight const light,
  float3 const pos_ws,
  float3 const normal_ws,
  float3 const dir_to_cam_ws,
  float3 const albedo,
  float const metallic,
  float const roughness
) {
  float3 dir_to_light_ws = light.position - pos_ws;
  float const dist = length(dir_to_light_ws);
  dir_to_light_ws = normalize(dir_to_light_ws);

  float const range_mul = float(dist <= light.range);
  float const theta_cos = dot(dir_to_light_ws, -light.direction);
  float const eps = light.half_inner_angle_cos - light.half_outer_angle_cos;
  float const intensity = saturate((theta_cos - light.half_outer_angle_cos) / eps);

  float3 lighting = CookTorrance(normal_ws, dir_to_cam_ws, dir_to_light_ws, albedo, metallic, roughness, light.color,
    light.intensity, CalculateAttenuation(dist));
  lighting *= intensity;
  lighting *= range_mul;

  return lighting;
}


float3 EvaluatePointLight(
  ShaderLight const light,
  float3 const pos_ws,
  float3 const normal_ws,
  float3 const dir_to_cam_ws,
  float3 const albedo,
  float const metallic,
  float const roughness
) {
  float3 dir_to_light_ws = light.position - pos_ws;
  float const dist = length(dir_to_light_ws);
  dir_to_light_ws = normalize(dir_to_light_ws);

  float const range_mul = float(dist <= light.range);

  float3 lighting = CookTorrance(normal_ws, dir_to_cam_ws, dir_to_light_ws, albedo, metallic, roughness, light.color,
    light.intensity, CalculateAttenuation(dist));
  lighting *= range_mul;

  return lighting;
}


void CalculateShadowSamplingCoordinates(
  float3 const pos_ws,
  float3 const normal_ws,
  float const shadow_map_texel_size,
  float const depth_bias,
  float const normal_bias,
  float4x4 const view_proj_mtx,
  out float2 uv,
  out float depth
) {
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


float SampleShadowCascadeFromAtlas(
  ShaderPositionalLightShadow const shadow,
  Texture2D<float> const atlas,
  SamplerComparisonState const shadow_samp,
  uint const shadow_idx,
  float3 const pos_ws,
  float3 const normal_ws,
  int const shadow_filtering_mode
) {
  uint2 atlas_size;
  atlas.GetDimensions(atlas_size.x, atlas_size.y);
  float2 const atlas_texel_size = 1.0 / atlas_size;
  float2 const shadow_map_texel_size = atlas_texel_size / shadow.atlas_scales[shadow_idx];
  float const shadow_map_bias_texel_size = max(shadow_map_texel_size.x, shadow_map_texel_size.y);

  float2 uv;
  float depth;
  CalculateShadowSamplingCoordinates(pos_ws, normal_ws, shadow_map_bias_texel_size, shadow.depth_bias,
    shadow.normal_bias, shadow.view_proj_matrices[shadow_idx], uv, depth);

  uv *= shadow.atlas_scales[shadow_idx];
  uv += shadow.atlas_offsets[shadow_idx];

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


float SampleShadowCascadeFromArray(
  ShaderDirectionalLightShadow const shadow,
  Texture2DArray<float> const shadow_map_array,
  SamplerComparisonState const shadow_samp,
  uint const cascade_idx,
  float3 const pos_ws,
  float3 const normal_ws,
  int const shadow_filtering_mode
) {
  float const shadow_map_texel_size = GetShadowMapArrayTexelSize(shadow_map_array).x;

  float2 uv;
  float depth;
  CalculateShadowSamplingCoordinates(pos_ws, normal_ws, shadow_map_texel_size, shadow.depth_bias, shadow.normal_bias,
    shadow.view_proj_matrices[cascade_idx], uv, depth);

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


float EvaluateDirectionalShadow(
  ShaderDirectionalLightShadow const shadow,
  Texture2DArray<float> const shadow_map_arr,
  SamplerComparisonState const shadow_samp,
  int const shadow_filtering_mode,
  uint const cascade_count,
  float3 const pos_ws,
  float3 const normal_ws,
  float const pos_vs_z
) {
  uint const cascade_idx = ComputeCascadeIdx(pos_vs_z, shadow.split_distances);

  return cascade_idx < cascade_count
           ? SampleShadowCascadeFromArray(shadow, shadow_map_arr, shadow_samp, cascade_idx, pos_ws, normal_ws,
             shadow_filtering_mode)
           : 1.0;
}


float EvaluateSpotLightShadow(
  ShaderPositionalLightShadow const shadow,
  Texture2D<float> const shadow_atlas,
  SamplerComparisonState const shadow_samp,
  int const shadow_filtering_mode,
  float3 const pos_ws,
  float3 const normal_ws
) {
  return SampleShadowCascadeFromAtlas(shadow, shadow_atlas, shadow_samp, 0, pos_ws, normal_ws, shadow_filtering_mode);
}


float EvaluatePointLightShadow(
  ShaderPositionalLightShadow const shadow,
  Texture2D<float> const shadow_atlas,
  SamplerComparisonState const shadow_samp,
  int const shadow_filtering_mode,
  float3 const pos_ws,
  float3 const normal_ws,
  float3 const light_pos_ws
) {
  float3 const dir_from_light_ws = pos_ws - light_pos_ws;

  uint max_idx = abs(dir_from_light_ws.x) > abs(dir_from_light_ws.y) ? 0 : 1;
  max_idx = abs(dir_from_light_ws[max_idx]) > abs(dir_from_light_ws.z) ? max_idx : 2;
  uint shadow_map_idx = max_idx * 2;

  if (sign(dir_from_light_ws[max_idx]) < 0) {
    shadow_map_idx += 1;
  }

  return (shadow.allocated_mask & (1 << shadow_map_idx)) != 0
           ? SampleShadowCascadeFromAtlas(shadow, shadow_atlas, shadow_samp, shadow_map_idx, pos_ws, normal_ws,
             shadow_filtering_mode)
           : 1.0;
}

#endif
