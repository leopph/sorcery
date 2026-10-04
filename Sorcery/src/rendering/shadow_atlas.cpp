#include "shadow_atlas.hpp"

#include "shaders/shader_interop.h"


namespace sorcery::rendering {
ShadowAtlas::ShadowAtlas(wand::GraphicsDevice* const device, DXGI_FORMAT const depth_format, UINT const size,
                         int const subdiv_size) :
  GridLike{subdiv_size},
  tex_{
    device->CreateTexture(
      wand::TextureDesc{
        wand::TextureDimension::k2D, size, size, 1, 1, depth_format, 1, true, false, true, false
      }, wand::CpuAccess::kNone,
      std::array{D3D12_CLEAR_VALUE{.Format = depth_format, .DepthStencil = {DEPTH_CLEAR_VALUE, 0}}}.data())
  },
  size_{size} {
  if (!IsPowerOfTwo(size_)) {
    throw std::runtime_error{"Shadow Atlas size must be power of 2."};
  }
}


ShadowAtlas::Cell::Cell(int const subdiv_size) :
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


auto ShadowAtlas::GetTex() const noexcept -> wand::SharedDeviceChildHandle<wand::Texture> const& {
  return tex_;
}


auto ShadowAtlas::GetSize() const noexcept -> UINT {
  return size_;
}
}
