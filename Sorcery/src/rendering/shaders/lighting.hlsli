#ifndef LIGHTING_HLSLI
#define LIGHTING_HLSLI

#include "brdf.hlsli"
#include "shader_interop.h"
#include "shadow_filtering_modes.h"
#include "shadow_sampling.hlsli"

uint CalculateShadowCascadeIdx(const float pos_vs_z, uniform const float4 cascade_splits)
{
  return (uint) dot(cascade_splits < pos_vs_z, 1.0);
}


float3 VisualizeShadowCascades(const float pos_vs_z, uniform const float4 cascade_splits,
                               uniform const uint cascade_count)
{
  const uint cascade_idx = CalculateShadowCascadeIdx(pos_vs_z, cascade_splits);

  if (cascade_idx >= cascade_count)
  {
    return float3(1, 1, 1);
  }

  float3 ret;

  switch (cascade_idx)
  {
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


float SampleShadowCascadeFromAtlas(const Texture2D<float> atlas, uniform const SamplerComparisonState shadow_samp,
                                   const ShaderLight light, const uint shadow_map_idx, const float3 pos_ws,
                                   const float3 normal_ws, uniform const int shadow_filtering_mode)
{
  uint atlas_size;
  atlas.GetDimensions(atlas_size, atlas_size);
  const float atlas_texel_size = 1.0 / atlas_size;
  const float shadow_map_texel_size = atlas_texel_size / light.shadowAtlasCellSizes[shadow_map_idx];

  const float4 pos_light_cs = mul(float4(pos_ws + normal_ws * shadow_map_texel_size * light.normalBias, 1),
    light.shadowViewProjMatrices[shadow_map_idx]);
  float3 pos_light_ndc = pos_light_cs.xyz / pos_light_cs.w;
  pos_light_ndc.xy = pos_light_ndc.xy * float2(0.5, -0.5) + 0.5;

  const float2 uv = pos_light_ndc.xy * light.shadowAtlasCellSizes[shadow_map_idx] + light.shadowAtlasCellOffsets[
                      shadow_map_idx];
  const float depth = pos_light_ndc.z - shadow_map_texel_size * -light.depthBias;

  switch (shadow_filtering_mode)
  {
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


float SampleShadowCascadeFromArray(const Texture2DArray<float> shadow_map_array,
                                   uniform const SamplerComparisonState shadow_samp, const ShaderLight light,
                                   const uint cascade_idx, const float3 pos_ws, const float3 normal_ws,
                                   uniform const int shadow_filtering_mode)
{
  const float shadow_map_texel_size = GetShadowMapArrayTexelSize(shadow_map_array).x;

  const float4 pos_light_cs = mul(float4(pos_ws + normal_ws * shadow_map_texel_size * light.normalBias, 1),
    light.shadowViewProjMatrices[cascade_idx]);
  const float3 pos_light_ndc = pos_light_cs.xyz / pos_light_cs.w;
  const float2 uv = pos_light_ndc.xy * float2(0.5, -0.5) + 0.5;
  const float depth = pos_light_ndc.z - shadow_map_texel_size * -light.depthBias;

  switch (shadow_filtering_mode)
  {
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


float CalculateAttenuation(const float distance)
{
  return 1 / pow(distance, 2);
}


float3 CalculateDirLight(const ShaderLight light, const float3 pos_ws, const float3 normal_ws,
                         const float3 dir_to_cam_ws, const float pos_vs_z, const float3 albedo, const float metallic,
                         const float roughness, const Texture2DArray<float> shadow_map_arr,
                         uniform const SamplerComparisonState shadow_samp, uniform const int shadow_filtering_mode,
                         uniform const float4 cascade_splits, uniform const uint cascade_count,
                         uniform const bool visualize_cascades)
{
  const float3 dir_to_light_ws = -light.direction;

  float3 lighting;

  [branch]
  if (visualize_cascades)
  {
    lighting = VisualizeShadowCascades(pos_vs_z, cascade_splits, cascade_count);
  }
  else
  {
    lighting = CookTorrance(normal_ws, dir_to_cam_ws, dir_to_light_ws, albedo, metallic, roughness, light.color,
      light.intensity, 1);
  }

  [branch]
  if (light.isCastingShadow)
  {
    const uint cascade_idx = CalculateShadowCascadeIdx(pos_vs_z, cascade_splits);

    [branch]
    if (cascade_idx < cascade_count)
    {
      lighting *= SampleShadowCascadeFromArray(shadow_map_arr, shadow_samp, light, cascade_idx, pos_ws, normal_ws,
        shadow_filtering_mode);
    }
  }

  return lighting;
}


float3 CalculateSpotLight(const ShaderLight light, const float3 pos_ws, const float3 normal_ws,
                          const float3 dir_to_cam_ws, const float3 albedo, const float metallic, const float roughness,
                          const Texture2D<float> shadow_atlas, uniform const SamplerComparisonState shadow_samp,
                          uniform const int shadow_filtering_mode)
{
  float3 dir_to_light_ws = light.position - pos_ws;
  const float dist = length(dir_to_light_ws);
  dir_to_light_ws = normalize(dir_to_light_ws);

  const float range_mul = float(dist <= light.range);
  const float theta_cos = dot(dir_to_light_ws, -light.direction);
  const float eps = light.halfInnerAngleCos - light.halfOuterAngleCos;
  const float intensity = saturate((theta_cos - light.halfOuterAngleCos) / eps);

  float3 lighting = CookTorrance(normal_ws, dir_to_cam_ws, dir_to_light_ws, albedo, metallic, roughness, light.color,
    light.intensity, CalculateAttenuation(dist));
  lighting *= intensity;
  lighting *= range_mul;

  [branch]
  if (light.isCastingShadow)
  {
    lighting *= SampleShadowCascadeFromAtlas(shadow_atlas, shadow_samp, light, 0, pos_ws, normal_ws,
      shadow_filtering_mode);
  }

  return lighting;
}


float3 CalculatePointLight(const ShaderLight light, const float3 pos_ws, const float3 normal_ws,
                           const float3 dir_to_cam_ws, const float3 albedo, const float metallic, const float roughness,
                           const Texture2D<float> shadow_atlas, uniform const SamplerComparisonState shadow_samp,
                           uniform const int shadow_filtering_mode)
{
  float3 dir_to_light_ws = light.position - pos_ws;
  const float dist = length(dir_to_light_ws);
  dir_to_light_ws = normalize(dir_to_light_ws);

  const float range_mul = float(dist <= light.range);

  float3 lighting = CookTorrance(normal_ws, dir_to_cam_ws, dir_to_light_ws, albedo, metallic, roughness, light.color,
    light.intensity, CalculateAttenuation(dist));
  lighting *= range_mul;

  [branch]
  if (light.isCastingShadow)
  {
    const float3 dir_from_light_ws = pos_ws - light.position;

    uint max_idx = abs(dir_from_light_ws.x) > abs(dir_from_light_ws.y) ? 0 : 1;
    max_idx = abs(dir_from_light_ws[max_idx]) > abs(dir_from_light_ws.z) ? max_idx : 2;
    uint shadow_map_idx = max_idx * 2;

    if (sign(dir_from_light_ws[max_idx]) < 0)
    {
      shadow_map_idx += 1;
    }

    [branch]
    if (light.sampleShadowMap[shadow_map_idx])
    {
      lighting *= SampleShadowCascadeFromAtlas(shadow_atlas, shadow_samp, light, shadow_map_idx, pos_ws, normal_ws,
        shadow_filtering_mode);
    }
  }

  return lighting;
}

#endif
