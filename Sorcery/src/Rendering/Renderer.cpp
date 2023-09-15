#include "Renderer.hpp"

#include "RendererImpl.hpp"

#include <cassert>


namespace sorcery {
Renderer gRenderer;


auto Renderer::StartUp() -> void {
  mImpl = new Impl{};
  mImpl->StartUp();
}


auto Renderer::ShutDown() -> void {
  mImpl->ShutDown();
  delete mImpl;
  mImpl = nullptr;
}


auto Renderer::DrawCamera(Camera const& cam, RenderTarget const* rt) -> void {
  assert(mImpl);
  mImpl->DrawCamera(cam, rt);
}


auto Renderer::DrawAllCameras(RenderTarget const* rt) -> void {
  assert(mImpl);
  mImpl->DrawAllCameras(rt);
}


auto Renderer::DrawLineAtNextRender(Vector3 const& from, Vector3 const& to, Color const& color) -> void {
  assert(mImpl);
  mImpl->DrawLineAtNextRender(from, to, color);
}


auto Renderer::DrawGizmos(RenderTarget const* const rt) -> void {
  assert(mImpl);
  mImpl->DrawGizmos(rt);
}


auto Renderer::ClearAndBindMainRt(ObserverPtr<ID3D11DeviceContext> const ctx) const noexcept -> void {
  assert(mImpl);
  mImpl->ClearAndBindMainRt(ctx);
}


auto Renderer::BlitMainRtToSwapChain(ObserverPtr<ID3D11DeviceContext> const ctx) const noexcept -> void {
  assert(mImpl);
  mImpl->BlitMainRtToSwapChain(ctx);
}


auto Renderer::Present() noexcept -> void {
  assert(mImpl);
  mImpl->Present();
}


auto Renderer::GetDevice() const noexcept -> ObserverPtr<ID3D11Device> {
  assert(mImpl);
  return mImpl->GetDevice();
}


auto Renderer::GetThreadContext() noexcept -> ObserverPtr<ID3D11DeviceContext> {
  assert(mImpl);
  return mImpl->GetThreadContext();
}


auto Renderer::ExecuteCommandList(ObserverPtr<ID3D11CommandList> const cmdList) noexcept -> void {
  assert(mImpl);
  mImpl->ExecuteCommandList(cmdList);
}


auto Renderer::ExecuteCommandList(ObserverPtr<ID3D11DeviceContext> const ctx) noexcept -> void {
  assert(mImpl);
  mImpl->ExecuteCommandList(ctx);
}


auto Renderer::GetTemporaryRenderTarget(RenderTarget::Desc const& desc) -> RenderTarget& {
  assert(mImpl);
  return mImpl->GetTemporaryRenderTarget(desc);
}


auto Renderer::GetDefaultMaterial() const noexcept -> ObserverPtr<Material> {
  assert(mImpl);
  return mImpl->GetDefaultMaterial();
}


auto Renderer::GetCubeMesh() const noexcept -> ObserverPtr<Mesh> {
  assert(mImpl);
  return mImpl->GetCubeMesh();
}


auto Renderer::GetPlaneMesh() const noexcept -> ObserverPtr<Mesh> {
  assert(mImpl);
  return mImpl->GetPlaneMesh();
}


auto Renderer::GetSyncInterval() const noexcept -> int {
  assert(mImpl);
  return mImpl->GetSyncInterval();
}


auto Renderer::SetSyncInterval(int interval) noexcept -> void {
  assert(mImpl);
  mImpl->SetSyncInterval(interval);
}


auto Renderer::GetInFlightFrameCount() const noexcept -> int {
  assert(mImpl);
  return mImpl->GetInFlightFrameCount();
}


auto Renderer::SetInFlightFrameCount(int const count) -> void {
  assert(mImpl);
  mImpl->SetInFlightFrameCount(count);
}


auto Renderer::GetMultisamplingMode() const noexcept -> MultisamplingMode {
  assert(mImpl);
  return mImpl->GetMultisamplingMode();
}


auto Renderer::SetMultisamplingMode(MultisamplingMode const mode) noexcept -> void {
  assert(mImpl);
  mImpl->SetMultisamplingMode(mode);
}


auto Renderer::IsDepthNormalPrePassEnabled() const noexcept -> bool {
  return mImpl->IsDepthNormalPrePassEnabled();
}


auto Renderer::SetDepthNormalPrePassEnabled(bool const enabled) noexcept -> void {
  mImpl->SetDepthNormalPrePassEnabled(enabled);
}


auto Renderer::GetShadowDistance() const noexcept -> float {
  assert(mImpl);
  return mImpl->GetShadowDistance();
}


auto Renderer::SetShadowDistance(float const shadowDistance) noexcept -> void {
  assert(mImpl);
  mImpl->SetShadowDistance(shadowDistance);
}


auto Renderer::GetMaxShadowCascadeCount() noexcept -> int {
  return Impl::GetMaxShadowCascadeCount();
}


auto Renderer::GetShadowCascadeCount() const noexcept -> int {
  assert(mImpl);
  return mImpl->GetShadowCascadeCount();
}


auto Renderer::SetShadowCascadeCount(int const cascadeCount) noexcept -> void {
  assert(mImpl);
  mImpl->SetShadowCascadeCount(cascadeCount);
}


auto Renderer::GetNormalizedShadowCascadeSplits() const noexcept -> std::span<float const> {
  assert(mImpl);
  return mImpl->GetNormalizedShadowCascadeSplits();
}


auto Renderer::SetNormalizedShadowCascadeSplit(int const idx, float const split) noexcept -> void {
  assert(mImpl);
  mImpl->SetNormalizedShadowCascadeSplit(idx, split);
}


auto Renderer::IsVisualizingShadowCascades() const noexcept -> bool {
  assert(mImpl);
  return mImpl->IsVisualizingShadowCascades();
}


auto Renderer::VisualizeShadowCascades(bool visualize) noexcept -> void {
  assert(mImpl);
  mImpl->VisualizeShadowCascades(visualize);
}


auto Renderer::GetShadowFilteringMode() const noexcept -> ShadowFilteringMode {
  assert(mImpl);
  return mImpl->GetShadowFilteringMode();
}


auto Renderer::SetShadowFilteringMode(ShadowFilteringMode filteringMode) noexcept -> void {
  assert(mImpl);
  mImpl->SetShadowFilteringMode(filteringMode);
}


auto Renderer::GetAmbientLightColor() const noexcept -> Vector3 const& {
  assert(mImpl);
  return mImpl->GetAmbientLightColor();
}


auto Renderer::SetAmbientLightColor(Vector3 const& color) noexcept -> void {
  assert(mImpl);
  mImpl->SetAmbientLightColor(color);
}


auto Renderer::GetSsaoParams() const noexcept -> SsaoParams const& {
  assert(mImpl);
  return mImpl->GetSsaoParams();
}


auto Renderer::SetSsaoParams(SsaoParams const& ssaoParams) noexcept -> void {
  assert(mImpl);
  mImpl->SetSsaoParams(ssaoParams);
}


auto Renderer::GetGamma() const noexcept -> float {
  assert(mImpl);
  return mImpl->GetGamma();
}


auto Renderer::SetGamma(float gamma) noexcept -> void {
  assert(mImpl);
  mImpl->SetGamma(gamma);
}


auto Renderer::Register(StaticMeshComponent const& staticMeshComponent) noexcept -> void {
  assert(mImpl);
  mImpl->Register(staticMeshComponent);
}


auto Renderer::Unregister(StaticMeshComponent const& staticMeshComponent) noexcept -> void {
  assert(mImpl);
  mImpl->Unregister(staticMeshComponent);
}


auto Renderer::Register(LightComponent const& lightComponent) noexcept -> void {
  assert(mImpl);
  mImpl->Register(lightComponent);
}


auto Renderer::Unregister(LightComponent const& lightComponent) noexcept -> void {
  assert(mImpl);
  mImpl->Unregister(lightComponent);
}


auto Renderer::Register(SkyboxComponent const& skyboxComponent) noexcept -> void {
  assert(mImpl);
  mImpl->Register(skyboxComponent);
}


auto Renderer::Unregister(SkyboxComponent const& skyboxComponent) noexcept -> void {
  assert(mImpl);
  mImpl->Unregister(skyboxComponent);
}


auto Renderer::Register(Camera const& cam) noexcept -> void {
  assert(mImpl);
  mImpl->Register(cam);
}


auto Renderer::Unregister(Camera const& cam) noexcept -> void {
  assert(mImpl);
  mImpl->Unregister(cam);
}
}
