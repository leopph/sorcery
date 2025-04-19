#include "shadow_atlas.hpp"

#include "shaders/shader_interop.h"


namespace sorcery::rendering {
ShadowAtlas::ShadowAtlas(graphics::GraphicsDevice* const device, DXGI_FORMAT const depth_format, UINT const size,
                         int const subdiv_size):
  GridLike{subdiv_size},
  tex_{
    device->CreateTexture(
      graphics::TextureDesc{
        graphics::TextureDimension::k2D, size, size, 1, 1, depth_format, 1, true, false, true, false
      }, graphics::CpuAccess::kNone,
      std::array{D3D12_CLEAR_VALUE{.Format = depth_format, .DepthStencil = {DEPTH_CLEAR_VALUE, 0}}}.data())
  },
  size_{size} {
  if (!IsPowerOfTwo(size_)) {
    throw std::runtime_error{"Shadow Atlas size must be power of 2."};
  }
}


ShadowAtlas::Cell::Cell(int const subdiv_size):
  GridLike{subdiv_size} {
  subcells_.resize(GetElementCount());
}


auto ShadowAtlas::Cell::GetSubcell(int const idx) const -> std::optional<Subcell> const& {
  ThrowIfIndexIsInvalid(idx);
  return subcells_[idx];
}


auto ShadowAtlas::Cell::GetSubcell(int const idx) -> std::optional<Subcell>& {
  return const_cast<std::optional<Subcell>&>(const_cast<Cell const*>(this)->GetSubcell(idx));
}


auto ShadowAtlas::Cell::Resize(int const subdiv_size) -> void {
  SetSubdivisionSize(subdiv_size);
  subcells_.resize(GetElementCount());
}


ShadowAtlas::~ShadowAtlas() = default;


auto ShadowAtlas::GetTex() const noexcept -> graphics::SharedDeviceChildHandle<graphics::Texture> const& {
  return tex_;
}


auto ShadowAtlas::GetSize() const noexcept -> UINT {
  return size_;
}


auto ShadowAtlas::SetLookUpInfo(std::span<ShaderLight> lights) const -> void {
  for (auto i = 0; i < GetElementCount(); i++) {
    auto const& cell{GetCell(i)};

    for (auto j = 0; j < cell.GetElementCount(); j++) {
      if (auto const& subcell{cell.GetSubcell(j)}) {
        lights[subcell->visibleLightIdxIdx].isCastingShadow = TRUE;
        lights[subcell->visibleLightIdxIdx].sampleShadowMap[subcell->shadowMapIdx] = TRUE;
        lights[subcell->visibleLightIdxIdx].shadowViewProjMatrices[subcell->shadowMapIdx] = subcell->shadowViewProjMtx;
        lights[subcell->visibleLightIdxIdx].shadowAtlasCellOffsets[subcell->shadowMapIdx] =
          GetNormalizedElementOffset(i) + cell.GetNormalizedElementOffset(j) * GetNormalizedElementSize();
        lights[subcell->visibleLightIdxIdx].shadowAtlasCellSizes[subcell->shadowMapIdx] =
          GetNormalizedElementSize() * cell.GetNormalizedElementSize();
      }
    }
  }
}
}
