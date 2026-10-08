#ifndef DEFERRED_LIGHTING_HLSLI
#define DEFERRED_LIGHTING_HLSLI

#include "common.hlsli"
#include "fullscreen_tri.hlsli"
#include "gbuffer_utils.hlsli"
#include "lighting.hlsli"
#include "shader_interop.h"
#include "utility.hlsli"


DECLARE_PARAMS(DeferredLightingDrawParams);


float4 PsMain(PsIn const ps_in) : SV_Target {
  SamplerState const point_clamp_samp = SamplerDescriptorHeap[g_params.point_clamp_samp_idx];
  Texture2D const gbuffer0 = ResourceDescriptorHeap[g_params.gbuffer0_idx];
  Texture2D<float2> const gbuffer1 = ResourceDescriptorHeap[g_params.gbuffer1_idx];
  Texture2D<float2> const gbuffer2 = ResourceDescriptorHeap[g_params.gbuffer2_idx];

  float3 albedo;
  float3 norm_ws;
  float roughness;
  float metallic;
  float ao;

  UnpackGBuffer0(gbuffer0.Sample(point_clamp_samp, ps_in.uv.xy), albedo, ao);
  UnpackGBuffer1(gbuffer1.Sample(point_clamp_samp, ps_in.uv.xy), norm_ws);
  UnpackGBuffer2(gbuffer2.Sample(point_clamp_samp, ps_in.uv.xy), roughness, metallic);

  Texture2D<float> const depth_tex = ResourceDescriptorHeap[g_params.depth_tex_idx];
  float const depth = depth_tex.Sample(point_clamp_samp, ps_in.uv.xy, 0, 0);

  Texture2D<float> const ssao_tex = ResourceDescriptorHeap[g_params.ssao_tex_idx];
  ao *= ssao_tex.Sample(point_clamp_samp, ps_in.uv).r;

  const ConstantBuffer<ShaderPerViewConstants> per_view_cb = ResourceDescriptorHeap[g_params.per_view_cb_idx];

  float4 const pos_ndc = float4(UvToNdc(ps_in.uv.xy), depth, 1);
  float4 pos4_ws = mul(pos_ndc, per_view_cb.invViewProjMtx);
  float3 const pos_ws = pos4_ws.xyz / pos4_ws.w;
  float3 const pos_vs = mul(float4(pos_ws, 1), per_view_cb.viewMtx).xyz;
  float3 const dir_to_cam_ws = normalize(per_view_cb.viewPos - pos_ws);

  const ConstantBuffer<ShaderPerFrameConstants> per_frame_cb = ResourceDescriptorHeap[g_params.per_frame_cb_idx];

  float3 ambient = per_frame_cb.ambient_light_color * albedo;

  if (g_params.irradiance_map_idx != INVALID_RES_IDX && g_params.prefiltered_env_map_idx != INVALID_RES_IDX) {
    TextureCube const irradiance_map = GetResource(g_params.irradiance_map_idx);
    SamplerState const bi_clamp_samp = GetSampler(g_params.bi_clamp_samp_idx);

    float3 const irradiance = irradiance_map.Sample(bi_clamp_samp, norm_ws).rgb;
    float3 const diffuse = irradiance * albedo;

    TextureCube const prefiltered_env_map = GetResource(g_params.prefiltered_env_map_idx);
    SamplerState const tri_clamp_samp = GetSampler(g_params.tri_clamp_samp_idx);

    Texture2D<float2> const brdf_lut = GetResource(g_params.brdf_integration_map_idx);

    float2 prefiltered_env_map_size;
    uint prefiltered_env_map_mip_count;
    prefiltered_env_map.GetDimensions(0, prefiltered_env_map_size.x, prefiltered_env_map_size.y,
      prefiltered_env_map_mip_count);

    float const n_dot_v = saturate(dot(norm_ws, dir_to_cam_ws));
    float3 const in_light_dir_ws = reflect(-dir_to_cam_ws, norm_ws);

    float3 const prefiltered_color = prefiltered_env_map.SampleLevel(
      tri_clamp_samp, in_light_dir_ws, roughness * (prefiltered_env_map_mip_count - 1)).rgb;
    float2 const env_brdf = brdf_lut.Sample(bi_clamp_samp, float2(n_dot_v, roughness));

    float3 const f0 = CalcF0(albedo, metallic);
    float3 const fresnel = FresnelSchlickRoughness(n_dot_v, f0, roughness);
    float3 const specular = prefiltered_color * (fresnel * env_brdf.x + env_brdf.y);

    float3 const diffuse_factor = (1 - fresnel) * (1 - metallic);

    ambient = (diffuse_factor * diffuse + specular);
  }

  float3 out_color = ambient * ao;

  ConstantBuffer < ShaderCameraLightingData>
    const cam_light_data =
      ResourceDescriptorHeap[g_params.cam_light_data_buf_idx];

  Texture2DArray<float> const shadow_arr = ResourceDescriptorHeap[g_params.dir_shadow_arr_idx];
  Texture2D<float> const shadow_atlas = ResourceDescriptorHeap[g_params.pos_shadow_atlas_idx];
  SamplerComparisonState const shadow_samp = SamplerDescriptorHeap[g_params.shadow_samp_idx];

  if (cam_light_data.visible_light_count > 0) {
    StructuredBuffer<ShaderVisibleLight> const visible_lights = ResourceDescriptorHeap[g_params.visible_light_buf_idx];
    StructuredBuffer<ShaderLight> const lights = ResourceDescriptorHeap[g_params.light_buf_idx];

    for (uint i = 0; i < cam_light_data.visible_light_count; i++) {
      ShaderVisibleLight const visible_light = visible_lights[cam_light_data.first_visible_light + i];
      ShaderLight const light = lights[visible_light.light_idx];

      float3 light_contrib;

      switch (light.type) {
        case LIGHT_DIRECTIONAL: {
          light_contrib = EvaluateDirectionalLight(light, norm_ws, dir_to_cam_ws, albedo, metallic, roughness);

          if (visible_light.light_idx == cam_light_data.dir_shadow.light_idx) {
            if (per_frame_cb.visualize_dir_shadow_cascades) {
              light_contrib = VisualizeShadowCascades(pos_vs.z, cam_light_data.dir_shadow.split_distances,
                per_frame_cb.dir_shadow_cascade_count);
            } else {
              light_contrib *= EvaluateDirectionalShadow(cam_light_data.dir_shadow, shadow_arr, shadow_samp,
                per_frame_cb.shadow_filtering_mode, per_frame_cb.dir_shadow_cascade_count, pos_ws, norm_ws, pos_vs.z);
            }
          }

          break;
        }

        case LIGHT_SPOT: {
          light_contrib = EvaluateSpotLight(light, pos_ws, norm_ws, dir_to_cam_ws, albedo, metallic, roughness);

          if (visible_light.positional_shadow_idx != INVALID_IDX) {
            StructuredBuffer<ShaderPositionalLightShadow> const pos_shadows = ResourceDescriptorHeap[g_params.
              pos_shadow_buf_idx];
            light_contrib *= EvaluateSpotLightShadow(pos_shadows[visible_light.positional_shadow_idx], shadow_atlas,
              shadow_samp, per_frame_cb.shadow_filtering_mode, pos_ws, norm_ws);
          }

          break;
        }

        case LIGHT_POINT: {
          light_contrib = EvaluatePointLight(light, pos_ws, norm_ws, dir_to_cam_ws, albedo, metallic, roughness);

          if (visible_light.positional_shadow_idx != INVALID_IDX) {
            StructuredBuffer<ShaderPositionalLightShadow> const pos_shadows = ResourceDescriptorHeap[g_params.
              pos_shadow_buf_idx];
            light_contrib *= EvaluatePointLightShadow(pos_shadows[visible_light.positional_shadow_idx], shadow_atlas,
              shadow_samp, per_frame_cb.shadow_filtering_mode, pos_ws, norm_ws, light.position);
          }

          break;
        }

        default:
          return float4(1, 0, 1, 1);
      }

      out_color += light_contrib;
    }
  }

  return float4(out_color, 1);
}


#endif
