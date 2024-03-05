#include "MeshVSOut.hlsli"
#include "ShaderInterop.h"
#include "BRDF.hlsli"
#include "ShadowFilteringModes.h"
#include "ShadowSampling.hlsli"


struct DrawParams {
  uint mtl_idx;
  uint per_view_cb_idx;
  uint per_frame_cb_idx;
  uint mtl_samp_idx;
  uint point_clamp_samp_idx;
  uint shadow_samp_idx;
  uint ssao_tex_idx;
  uint light_buf_idx;
  uint dir_shadow_arr_idx;
  uint punc_shadow_atlas_idx;
};


ConstantBuffer<DrawParams> g_draw_params : register(b0, space0);


int CalculateShadowCascadeIdx(const float pos_vs_z, uniform const float4 cascade_splits) {
  return dot(cascade_splits < pos_vs_z, 1.0);
}


float3 VisualizeShadowCascades(const float pos_vs_z, uniform const float4 cascade_splits,
                               uniform const uint cascade_count) {
  const int cascade_idx = CalculateShadowCascadeIdx(pos_vs_z, cascade_splits);

  if (cascade_idx >= cascade_count) {
    return float3(1, 1, 1);
  }

  float3 ret;

  switch (cascade_idx) {
    case 0: ret = float3(108, 110, 160);
      break;
    case 1: ret = float3(184, 216, 186);
      break;
    case 2: ret = float3(252, 221, 188);
      break;
    case 3: ret = float3(239, 149, 157);
      break;
    default: ret = float3(1, 1, 1); // This should never be reached
      break;
  }

  return pow(ret / 255.0, 2.2);
}


float SampleShadowCascadeFromAtlas(const Texture2D<float> atlas, uniform const SamplerComparisonState shadow_samp,
                                   const ShaderLight light, const uint shadow_map_idx, const float3 pos_ws,
                                   const float3 normal_ws, uniform const int shadow_filtering_mode,
                                   uniform const bool use_reversed_z) {
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
  const float depth = pos_light_ndc.z - shadow_map_texel_size * light.depthBias * (use_reversed_z ? -1 : 1);

  switch (shadow_filtering_mode) {
    case SHADOW_FILTERING_NONE: return SampleShadowMapNoFilter(atlas, shadow_samp, uv, depth);
    case SHADOW_FILTERING_HARDWARE_PCF: return SampleShadowMapHardwarePCF(atlas, shadow_samp, uv, depth);
    case SHADOW_FILTERING_PCF_3x3: return SampleShadowMapPCF3x34TapFast(atlas, shadow_samp, uv, depth);
    case SHADOW_FILTERING_PCF_TENT_3x3: return SampleShadowMapPCF3x3Tent4Tap(atlas, shadow_samp, uv, depth);
    case SHADOW_FILTERING_PCF_TENT_5x5: return SampleShadowMapPCF5x5Tent9Tap(atlas, shadow_samp, uv, depth);
    default: return 1.0;
  }
}


float SampleShadowCascadeFromArray(const Texture2DArray<float> shadow_map_array,
                                   uniform const SamplerComparisonState shadow_samp, const ShaderLight light,
                                   const uint cascade_idx, const float3 pos_ws, const float3 normal_ws,
                                   uniform const int shadow_filtering_mode, uniform const bool use_reversed_z) {
  const float shadow_map_texel_size = GetShadowMapArrayTexelSize(shadow_map_array).x;

  const float4 pos_light_cs = mul(float4(pos_ws + normal_ws * shadow_map_texel_size * light.normalBias, 1),
    light.shadowViewProjMatrices[cascade_idx]);
  const float3 pos_light_ndc = pos_light_cs.xyz / pos_light_cs.w;
  const float2 uv = pos_light_ndc.xy * float2(0.5, -0.5) + 0.5;
  const float depth = pos_light_ndc.z - shadow_map_texel_size * light.depthBias * (use_reversed_z ? -1 : 1);

  switch (shadow_filtering_mode) {
    case SHADOW_FILTERING_NONE: return SampleShadowMapArrayNoFilter(shadow_map_array, shadow_samp, uv, cascade_idx,
        depth);
    case SHADOW_FILTERING_HARDWARE_PCF: return SampleShadowMapArrayHardwarePCF(shadow_map_array, shadow_samp, uv,
        cascade_idx, depth);
    case SHADOW_FILTERING_PCF_3x3: return SampleShadowMapArrayPCF3x34TapFast(shadow_map_array, shadow_samp, uv,
        cascade_idx, depth);
    case SHADOW_FILTERING_PCF_TENT_3x3: return SampleShadowMapArrayPCF3x3Tent4Tap(shadow_map_array, shadow_samp, uv,
        cascade_idx, depth);
    case SHADOW_FILTERING_PCF_TENT_5x5: return SampleShadowMapArrayPCF5x5Tent9Tap(shadow_map_array, shadow_samp, uv,
        cascade_idx, depth);
    default: return 1.0;
  }
}


float CalculateAttenuation(const float distance) {
  return 1 / pow(distance, 2);
}


float3 CalculateDirLight(const ShaderLight light, const float3 pos_ws, const float3 normal_ws,
                         const float3 dir_to_cam_ws, const float pos_vs_z, const float3 albedo, const float metallic,
                         const float roughness, const Texture2DArray<float> shadow_map_arr,
                         uniform const SamplerComparisonState shadow_samp, uniform const int shadow_filtering_mode,
                         uniform const float4 cascade_splits, uniform const uint cascade_count,
                         uniform const bool visualize_cascades, uniform const int use_reversed_z) {
  const float3 dir_to_light_ws = -light.direction;

  float3 lighting;

  [branch] if (visualize_cascades) {
    lighting = VisualizeShadowCascades(pos_vs_z, cascade_splits, cascade_count);
  } else {
    lighting = CookTorrance(normal_ws, dir_to_cam_ws, dir_to_light_ws, albedo, metallic, roughness, light.color,
      light.intensity, 1);
  }

  [branch] if (light.isCastingShadow) {
    const int cascade_idx = CalculateShadowCascadeIdx(pos_vs_z, cascade_splits);

    [branch] if (cascade_idx < cascade_count) {
      lighting *= SampleShadowCascadeFromArray(shadow_map_arr, shadow_samp, light, cascade_idx, pos_ws, normal_ws,
        shadow_filtering_mode, use_reversed_z);
    }
  }

  return lighting;
}


float3 CalculateSpotLight(const ShaderLight light, const float3 pos_ws, const float3 normal_ws,
                          const float3 dir_to_cam_ws, const float3 albedo, const float metallic, const float roughness,
                          const Texture2D<float> shadow_atlas, uniform const SamplerComparisonState shadow_samp,
                          uniform const int shadow_filtering_mode, uniform const bool use_reversed_z) {
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

  [branch] if (light.isCastingShadow) {
    lighting *= SampleShadowCascadeFromAtlas(shadow_atlas, shadow_samp, light, 0, pos_ws, normal_ws,
      shadow_filtering_mode, use_reversed_z);
  }

  return lighting;
}


float3 CalculatePointLight(const ShaderLight light, const float3 pos_ws, const float3 normal_ws,
                           const float3 dir_to_cam_ws, const float3 albedo, const float metallic, const float roughness,
                           const Texture2D<float> shadow_atlas, uniform const SamplerComparisonState shadow_samp,
                           uniform const int shadow_filtering_mode, uniform const bool use_reversed_z) {
  float3 dir_to_light_ws = light.position - pos_ws;
  const float dist = length(dir_to_light_ws);
  dir_to_light_ws = normalize(dir_to_light_ws);

  const float range_mul = float(dist <= light.range);

  float3 lighting = CookTorrance(normal_ws, dir_to_cam_ws, dir_to_light_ws, albedo, metallic, roughness, light.color,
    light.intensity, CalculateAttenuation(dist));
  lighting *= range_mul;

  [branch] if (light.isCastingShadow) {
    const float3 dir_from_light_ws = pos_ws - light.position;

    uint max_idx = abs(dir_from_light_ws.x) > abs(dir_from_light_ws.y) ? 0 : 1;
    max_idx = abs(dir_from_light_ws[max_idx]) > abs(dir_from_light_ws.z) ? max_idx : 2;
    uint shadow_map_idx = max_idx * 2;

    if (sign(dir_from_light_ws[max_idx]) < 0) {
      shadow_map_idx += 1;
    }

    [branch] if (light.sampleShadowMap[shadow_map_idx]) {
      lighting *= SampleShadowCascadeFromAtlas(shadow_atlas, shadow_samp, light, shadow_map_idx, pos_ws, normal_ws,
        shadow_filtering_mode, use_reversed_z);
    }
  }

  return lighting;
}


float4 main(const MeshVsOut vs_out) : SV_Target {
  const ConstantBuffer<ShaderMaterial> mtl = ResourceDescriptorHeap[g_draw_params.mtl_idx];
  const SamplerState mtl_samp = SamplerDescriptorHeap[g_draw_params.mtl_samp_idx];

  if (mtl.blendMode == BLEND_MODE_ALPHA_CLIP && mtl.opacity_map_idx != INVALID_RES_IDX) {
    const Texture2D<float> opacity_map = ResourceDescriptorHeap[mtl.opacity_map_idx];

    if (opacity_map.Sample(mtl_samp, vs_out.uv) < mtl.alphaThreshold) {
      discard;
    }
  }

  float3 albedo = mtl.albedo;

  if (mtl.albedo_map_idx != INVALID_RES_IDX) {
    const Texture2D albedo_map = ResourceDescriptorHeap[mtl.albedo_map_idx];
    albedo *= albedo_map.Sample(mtl_samp, vs_out.uv).rgb;
  }

  float metallic = mtl.metallic;

  if (mtl.metallic_map_idx != INVALID_RES_IDX) {
    const Texture2D<float> metallic_map = ResourceDescriptorHeap[mtl.metallic_map_idx];
    metallic *= metallic_map.Sample(mtl_samp, vs_out.uv).r;
  }

  float roughness = mtl.roughness;

  if (mtl.roughness_map_idx != INVALID_RES_IDX) {
    const Texture2D<float> roughness_map = ResourceDescriptorHeap[mtl.roughness_map_idx];
    roughness *= roughness_map.Sample(mtl_samp, vs_out.uv).r;
  }

  const ConstantBuffer<ShaderPerFrameConstants> per_frame_cb = ResourceDescriptorHeap[g_draw_params.per_frame_cb_idx];
  const float2 screen_uv = vs_out.positionCS.xy / per_frame_cb.screenSize;

  const SamplerState point_clamp_samp = SamplerDescriptorHeap[g_draw_params.point_clamp_samp_idx];
  const Texture2D<float> ssao_tex = ResourceDescriptorHeap[g_draw_params.ssao_tex_idx];
  float ao = mtl.ao * ssao_tex.Sample(point_clamp_samp, screen_uv).r;

  if (mtl.ao_map_idx != INVALID_RES_IDX) {
    const Texture2D<float> ao_map = ResourceDescriptorHeap[mtl.ao_map_idx];
    ao *= ao_map.Sample(mtl_samp, vs_out.uv).r;
  }

  float3 normal = normalize(vs_out.normalWS);

  if (mtl.normal_map_idx != INVALID_RES_IDX) {
    const Texture2D<float3> normal_map = ResourceDescriptorHeap[mtl.normal_map_idx];
    normal = normal_map.Sample(mtl_samp, vs_out.uv).rgb;
    normal *= 2.0;
    normal -= 1.0;
    normal = normalize(mul(normalize(normal), vs_out.tbnMtxWS));
  }

  const ConstantBuffer<ShaderPerViewConstants> per_view_cb = ResourceDescriptorHeap[g_draw_params.per_view_cb_idx];
  const float3 dir_to_cam_ws = normalize(per_view_cb.viewPos - vs_out.positionWS);

  float3 out_color = per_frame_cb.ambientLightColor * albedo * ao;

  uint light_count;
  uint _;
  const StructuredBuffer<ShaderLight> lights = ResourceDescriptorHeap[g_draw_params.light_buf_idx];
  lights.GetDimensions(light_count, _);

  const Texture2DArray<float> dir_light_shadow_map_arr = ResourceDescriptorHeap[g_draw_params.dir_shadow_arr_idx];
  const Texture2D<float> punc_light_shadow_atlas = ResourceDescriptorHeap[g_draw_params.punc_shadow_atlas_idx];
  const SamplerComparisonState shadow_samp = SamplerDescriptorHeap[g_draw_params.shadow_samp_idx];

  for (uint i = 0; i < light_count; i++) {
    if (lights[i].type == 0) {
      out_color += CalculateDirLight(lights[i], vs_out.positionWS, vs_out.normalWS, dir_to_cam_ws, vs_out.positionVS.z,
        albedo, metallic, roughness, dir_light_shadow_map_arr, shadow_samp, per_frame_cb.shadowFilteringMode,
        per_view_cb.shadowCascadeSplitDistances, per_frame_cb.shadowCascadeCount, per_frame_cb.visualizeShadowCascades,
        per_frame_cb.isUsingReversedZ);
    } else if (lights[i].type == 1) {
      out_color += CalculateSpotLight(lights[i], vs_out.positionWS, vs_out.normalWS, dir_to_cam_ws, albedo, metallic,
        roughness, punc_light_shadow_atlas, shadow_samp, per_frame_cb.shadowFilteringMode,
        per_frame_cb.isUsingReversedZ);
    } else if (lights[i].type == 2) {
      out_color += CalculatePointLight(lights[i], vs_out.positionWS, vs_out.normalWS, dir_to_cam_ws, albedo, metallic,
        roughness, punc_light_shadow_atlas, shadow_samp, per_frame_cb.shadowFilteringMode,
        per_frame_cb.isUsingReversedZ);
    }
  }

  return float4(out_color, 1);
}
