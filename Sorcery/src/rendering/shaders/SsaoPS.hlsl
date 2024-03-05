#include "ShaderInterop.h"
#include "ScreenVsOut.hlsli"
#include "Util.hlsli"

struct DrawParams {
  uint noise_tex_idx;
  uint depth_tex_idx;
  uint normal_tex_idx;
  uint samp_buf_idx;
  uint point_clamp_samp_idx;
  uint point_wrap_samp_idx;
  uint ssao_cb_idx;
  uint per_view_cb_idx;
  uint per_frame_cb_idx;
};

ConstantBuffer<DrawParams> g_draw_params : register(b0, space0);

float3 CalculatePositionVsAtUv(const Texture2D<float> depth_tex, const SamplerState point_clamp_samp,
                               const ConstantBuffer<ShaderPerViewConstants> per_view_cb, const float2 uv) {
  const float depth = depth_tex.Sample(point_clamp_samp, uv).r;
  const float4 pos_vs = mul(float4(UvToNdc(uv), depth, 1), per_view_cb.invProjMtx);
	return pos_vs.xyz / pos_vs.w;
}

float main(const ScreenVsOut vs_out) : SV_Target {
	float2 noise_tex_size;

  const Texture2D<float3> noise_tex = ResourceDescriptorHeap[g_draw_params.noise_tex_idx];
  noise_tex.GetDimensions(noise_tex_size.x, noise_tex_size.y);

  const ConstantBuffer<ShaderPerFrameConstants> per_frame_cb = ResourceDescriptorHeap[g_draw_params.per_frame_cb_idx];
  const float2 noise_scale = per_frame_cb.screenSize / noise_tex_size;

  const SamplerState point_wrap_samp = SamplerDescriptorHeap[g_draw_params.point_wrap_samp_idx];
  const float3 noise = normalize(noise_tex.Sample(point_wrap_samp, vs_out.uv * noise_scale).xyz);

  const Texture2D<float> depth_tex = ResourceDescriptorHeap[g_draw_params.depth_tex_idx];
  const SamplerState point_clamp_samp = SamplerDescriptorHeap[g_draw_params.point_clamp_samp_idx];
  const ConstantBuffer<ShaderPerViewConstants> per_view_cb = ResourceDescriptorHeap[g_draw_params.per_view_cb_idx];
	const float3 hemisphere_origin_vs = CalculatePositionVsAtUv(depth_tex, point_clamp_samp, per_view_cb, vs_out.uv);

  const Texture2D<float3> normal_tex = ResourceDescriptorHeap[g_draw_params.normal_tex_idx];
  const float3 normal_vs = normalize(mul(float4(normal_tex.Sample(point_clamp_samp, vs_out.uv).xyz, 0), per_view_cb.viewMtx).xyz);
	const float3 tangent_vs = normalize(noise - normal_vs * dot(noise, normal_vs));
	const float3 bitangent_vs = cross(normal_vs, tangent_vs);
	const float3x3 tbn_mtx_vs = float3x3(tangent_vs, bitangent_vs, normal_vs);

	float occlusion = 0.0;
	uint sample_count, stride;

  const StructuredBuffer<float4> samples = ResourceDescriptorHeap[g_draw_params.samp_buf_idx];
  samples.GetDimensions(sample_count, stride);

  const ConstantBuffer<ShaderSsaoConstants> ssao_cb = ResourceDescriptorHeap[g_draw_params.ssao_cb_idx];

	for (uint i = 0; i < sample_count; i++) {
    const float3 samplePos = mul(samples[i].xyz, tbn_mtx_vs) * ssao_cb.radius + hemisphere_origin_vs;

		float4 sampleOffset = mul(float4(samplePos, 1), per_view_cb.projMtx);
		sampleOffset /= sampleOffset.w;

		const float sampleDepth = CalculatePositionVsAtUv(depth_tex, point_clamp_samp, per_view_cb, NdcToUv(sampleOffset.xy)).z;

    const float rangeCheck = smoothstep(0.0, 1.0, ssao_cb.radius / abs(hemisphere_origin_vs.z - sampleDepth));
    occlusion += step(sampleDepth, samplePos.z - ssao_cb.bias) * rangeCheck;
  }

  return pow(1 - occlusion / sample_count, ssao_cb.power);
}
