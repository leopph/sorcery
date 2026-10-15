#include "ssao_pass.hpp"

#include <array>
#include <bit>
#include <cmath>
#include <random>
#include <span>
#include <stdexcept>

#include "wand/command_list_helpers.hpp"

#include "rendering/temporary_render_target_pool.hpp"
#include "rendering/shaders/shader_interop.h"
#include "sorcery/rendering/render_target.hpp"

#ifndef NDEBUG
#include "rendering/shaders/generated/Debug/ssao_blur_ps.h"
#include "rendering/shaders/generated/Debug/ssao_main_ps.h"
#include "rendering/shaders/generated/Debug/ssao_vs.h"
#else
#include "rendering/shaders/generated/Release/ssao_blur_ps.h"
#include "rendering/shaders/generated/Release/ssao_main_ps.h"
#include "rendering/shaders/generated/Release/ssao_vs.h"
#endif


namespace sorcery::rendering {
SsaoPass::SsaoPass(wand::GraphicsDevice& device, TemporaryRenderTargetPool& rt_pool) :
  device_{&device},
  rt_pool_{&rt_pool} {
  noise_tex_ = device_->CreateTexture(wand::TextureDesc{
    wand::TextureDimension::k2D, SSAO_NOISE_TEX_DIM, SSAO_NOISE_TEX_DIM, 1, 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1,
    false, false, true, false
  }, wand::CpuAccess::kNone, nullptr);
  noise_tex_->SetDebugName(L"SSAO Noise");
}


auto SsaoPass::ExtractFrame(FrameData& data, SsaoParams const& params) -> void {
  if (params.sample_count < 1) {
    throw std::runtime_error{"Invalid SSAO sample count."};
  }

  auto const samples_changed = current_sample_count_ != params.sample_count;

  if (samples_changed) {
    if (!samples_buf_ || samples_buf_->GetDesc().size < params.sample_count * sizeof(Vector4)) {
      samples_buf_ = wand::CreateBufferWithView(*device_, wand::BufferDesc{
        .size = params.sample_count * sizeof(Vector4),
        .usage = wand::BufferUsage::kShaderResource | wand::BufferUsage::kCopyDestination
      }, wand::BufferViewDesc{
        .offset = 0,
        .size = params.sample_count * sizeof(Vector4),
        .stride = sizeof(Vector4),
        .usage = wand::BufferViewUsage::kShaderResource
      }, wand::CpuAccess::kNone);
    }

    current_sample_count_ = params.sample_count;
  }

  data.gen_samples_count = samples_changed ? std::make_optional(params.sample_count) : std::nullopt;
  data.samples_buf = samples_buf_;
  data.ssao_pso = ssao_pso_;
  data.blur_pso = blur_pso_;
  data.params = params;
  data.upload_noise = !noise_uploaded_;
  noise_uploaded_ = true;
}


auto SsaoPass::PrepareFrame(
  wand::CommandList& cmd,
  RenderFrame& frame,
  FrameData const& data
) const -> void {
  if (data.gen_samples_count) {
    auto const samples = GenerateSamples(*data.gen_samples_count);
    frame.GetUploadArena().UploadBuffer(cmd, data.samples_buf, 0, as_bytes(std::span{samples}));
  }

  if (data.upload_noise) {
    std::vector<Vector4> ssao_noise;
    std::uniform_real_distribution dist{0.0f, 1.0f};
    std::default_random_engine gen; // NOLINT(cert-msc51-cpp)

    for (auto i{0}; i < SSAO_NOISE_TEX_DIM * SSAO_NOISE_TEX_DIM; i++) {
      ssao_noise.emplace_back(dist(gen) * 2 - 1, dist(gen) * 2 - 1, 0, 0);
    }

    frame.GetUploadArena().UploadTexture(cmd, noise_tex_, 0, std::array{
      D3D12_SUBRESOURCE_DATA{
        .pData = ssao_noise.data(),
        .RowPitch = SSAO_NOISE_TEX_DIM * sizeof(Vector4),
        .SlicePitch = SSAO_NOISE_TEX_DIM * SSAO_NOISE_TEX_DIM * sizeof(Vector4)
      }
    });

    cmd.Seal(*noise_tex_);
  }
}


auto SsaoPass::Record(
  wand::CommandList& cmd,
  RenderFrame const& frame,
  FrameData const& data,
  ViewContext const& view_ctx,
  Input const& input
) const -> Output {
  // Anything Extract can replace goes through FrameData, and only immutable state is read from members

  wand::ScopedEvent const event{cmd, L"SSAO"};

  auto const ssao_rt = rt_pool_->Acquire(RenderTarget::Desc{
    .width = view_ctx.width,
    .height = view_ctx.height,
    .color_format = kBufferFormat,
    .depth_stencil_format = std::nullopt,
    .sample_count = 1,
    .debug_name = L"SSAO RT"
  }, frame);

  if (!ssao_rt) {
    throw std::runtime_error{"Failed to acquire SSAO RT."};
  }

  {
    wand::ScopedEvent const e{cmd, L"Main"};
    cmd.SetPipelineState(*data.ssao_pso);
    cmd.SetShaderResource(PIPELINE_PARAM_INDEX(SsaoDrawParams, noise_tex_idx), *noise_tex_);
    cmd.SetShaderResource(PIPELINE_PARAM_INDEX(SsaoDrawParams, depth_tex_idx), *input.depth_sample_tex);
    cmd.SetShaderResource(PIPELINE_PARAM_INDEX(SsaoDrawParams, gbuffer1_tex_idx), *input.gbuffer1_tex);
    cmd.SetShaderResource(PIPELINE_PARAM_INDEX(SsaoDrawParams, samp_buf_idx), *data.samples_buf);
    cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(SsaoDrawParams, point_clamp_samp_idx), view_ctx.samp_point_clamp);
    cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(SsaoDrawParams, point_wrap_samp_idx), view_ctx.samp_point_wrap);
    cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(SsaoDrawParams, radius), std::bit_cast<UINT>(data.params.radius));
    cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(SsaoDrawParams, bias), std::bit_cast<UINT>(data.params.bias));
    cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(SsaoDrawParams, power), std::bit_cast<UINT>(data.params.power));
    cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(SsaoDrawParams, sample_count), data.params.sample_count);
    cmd.SetConstantBuffer(PIPELINE_PARAM_INDEX(SsaoDrawParams, per_view_cb_idx), *view_ctx.per_view_cb);
    cmd.SetConstantBuffer(PIPELINE_PARAM_INDEX(SsaoDrawParams, per_frame_cb_idx), *view_ctx.per_frame_cb);
    cmd.SetRenderTargets(std::span{
      std::array{static_cast<wand::Texture const*>(ssao_rt->GetColorTex().get())}.data(), 1
    }, nullptr);
    cmd.ClearRenderTarget(*ssao_rt->GetColorTex(), std::array{0.0f, 0.0f, 0.0f, 1.0f}, {});
    cmd.DrawInstanced(3, 1, 0, 0);
  }

  auto const ssao_blur_rt = rt_pool_->Acquire([&ssao_rt] {
    auto ret{ssao_rt->GetDesc()};
    ret.debug_name = L"SSAO Blur RT";
    return ret;
  }(), frame);

  if (!ssao_blur_rt) {
    throw std::runtime_error{"Failed to acquire SSAO Blur RT."};
  }

  {
    wand::ScopedEvent const e{cmd, L"Blur"};
    cmd.SetPipelineState(*data.blur_pso);
    cmd.SetShaderResource(PIPELINE_PARAM_INDEX(SsaoBlurDrawParams, in_tex_idx),
      *ssao_rt->GetColorTex());
    cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(SsaoBlurDrawParams, point_clamp_samp_idx), view_ctx.samp_point_clamp);
    cmd.SetRenderTargets(std::span{
      std::array{static_cast<wand::Texture const*>(ssao_blur_rt->GetColorTex().get())}.data(), 1
    }, nullptr);
    cmd.ClearRenderTarget(*ssao_blur_rt->GetColorTex(), std::array{0.0f, 0.0f, 0.0f, 1.0f}, {});
    cmd.DrawInstanced(3, 1, 0, 0);
  }

  return Output{
    .ssao_tex = MakeObserver(ssao_blur_rt->GetColorTex().get())
  };
}


auto SsaoPass::RecreatePipelines() -> void {
  CD3DX12_DEPTH_STENCIL_DESC1 const depth_stencil_disabled{
    FALSE, D3D12_DEPTH_WRITE_MASK_ZERO, D3D12_COMPARISON_FUNC_NONE, FALSE, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, FALSE
  };

  CD3DX12_RT_FORMAT_ARRAY const format_arr{
    D3D12_RT_FORMAT_ARRAY{.RTFormats = {kBufferFormat}, .NumRenderTargets = 1}
  };

  wand::PipelineDesc const ssao_pso_desc{
    .vs = CD3DX12_SHADER_BYTECODE{&g_ssao_vs_bytes, ARRAYSIZE(g_ssao_vs_bytes)},
    .ps = CD3DX12_SHADER_BYTECODE{&g_ssao_main_ps_bytes, ARRAYSIZE(g_ssao_main_ps_bytes)},
    .depth_stencil_state = depth_stencil_disabled, .rt_formats = format_arr,
  };

  ssao_pso_ = device_->CreatePipelineState(ssao_pso_desc, sizeof(SsaoDrawParams) / 4);

  wand::PipelineDesc const ssao_blur_pso_desc{
    .vs = CD3DX12_SHADER_BYTECODE{&g_ssao_vs_bytes, ARRAYSIZE(g_ssao_vs_bytes)},
    .ps = CD3DX12_SHADER_BYTECODE{&g_ssao_blur_ps_bytes, ARRAYSIZE(g_ssao_blur_ps_bytes)},
    .depth_stencil_state = depth_stencil_disabled, .rt_formats = format_arr
  };

  blur_pso_ = device_->CreatePipelineState(ssao_blur_pso_desc, sizeof(SsaoBlurDrawParams) / 4);
}


auto SsaoPass::GenerateSamples(uint32_t const sample_count) -> std::vector<Vector4> {
  std::vector<Vector4> samples(sample_count);

  std::uniform_real_distribution dist{0.0f, 1.0f};
  std::default_random_engine gen; // NOLINT(cert-msc51-cpp)

  for (auto i = 0uz; i < sample_count; i++) {
    Vector3 sample{dist(gen) * 2 - 1, dist(gen) * 2 - 1, dist(gen)};
    Normalize(sample);
    sample *= dist(gen);

    auto scale{static_cast<float>(i) / static_cast<float>(sample_count)};
    scale = std::lerp(0.1f, 1.0f, scale * scale);
    sample *= scale;

    samples[i] = Vector4{sample, 0};
  }

  return samples;
}
}
