#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "wand/wand.hpp"

#include "rendering/ssao_params.hpp"
#include "rendering/view_context.hpp"
#include "sorcery/Math.hpp"
#include "sorcery/observer_ptr.hpp"


namespace sorcery::rendering {
class RenderFrame;
class TemporaryRenderTargetPool;


class SsaoPass {
public:
  struct FrameData {
    wand::SharedDeviceHandle<wand::PipelineState> ssao_pso;
    wand::SharedDeviceHandle<wand::PipelineState> blur_pso;
    wand::SharedDeviceHandle<wand::BufferView> samples_buf;
    SsaoParams params;
    std::optional<std::uint32_t> gen_samples_count;
    bool upload_noise;
  };


  struct Input {
    ObserverPtr<wand::Texture> depth_sample_tex;
    ObserverPtr<wand::Texture> gbuffer1_tex;
  };


  struct Output {
    ObserverPtr<wand::Texture> ssao_tex;
  };


  SsaoPass(wand::GraphicsDevice& device, TemporaryRenderTargetPool& rt_pool);


  auto ExtractFrame(
    FrameData& data,
    SsaoParams const& params
  ) -> void;

  auto PrepareFrame(
    wand::CommandList& cmd,
    RenderFrame& frame,
    FrameData const& data
  ) const -> void;

  [[nodiscard]]
  auto Record(
    wand::CommandList& cmd,
    RenderFrame const& frame,
    FrameData const& data,
    ViewContext const& view_ctx,
    Input const& input
  ) const -> Output;

  auto RecreatePipelines() -> void;

private:
  static
  auto GenerateSamples(uint32_t sample_count) -> std::vector<Vector4>;

  ObserverPtr<wand::GraphicsDevice> device_;
  ObserverPtr<TemporaryRenderTargetPool> rt_pool_;

  wand::SharedDeviceHandle<wand::PipelineState> ssao_pso_;
  wand::SharedDeviceHandle<wand::PipelineState> blur_pso_;

  wand::SharedDeviceHandle<wand::Texture> noise_tex_;
  wand::SharedDeviceHandle<wand::BufferView> samples_buf_;

  std::optional<std::uint32_t> current_sample_count_;

  bool noise_uploaded_{false};

  static DXGI_FORMAT constexpr kBufferFormat{DXGI_FORMAT_R8_UNORM};
};
}
