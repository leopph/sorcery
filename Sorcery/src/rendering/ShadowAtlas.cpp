#include "ShadowAtlas.hpp"

#include "shaders\shader_interop.h"


namespace sorcery {
ShadowAtlas::ShadowAtlas(ID3D11Device* const device, int const size, int const subdivSize):
  GridLike{subdivSize},
  mSize{size} {
  if (!IsPowerOfTwo(mSize)) {
    throw std::runtime_error{"Shadow Atlas size must be power of 2."};
  }

  D3D11_TEXTURE2D_DESC const texDesc{
    .Width = static_cast<UINT>(size),
    .Height = static_cast<UINT>(size),
    .MipLevels = 1,
    .ArraySize = 1,
    .Format = DXGI_FORMAT_R32_TYPELESS,
    .SampleDesc = {.Count = 1, .Quality = 0},
    .Usage = D3D11_USAGE_DEFAULT,
    .BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE,
    .CPUAccessFlags = 0,
    .MiscFlags = 0
  };

  if (FAILED(device->CreateTexture2D(&texDesc, nullptr, mTex.GetAddressOf()))) {
    throw std::runtime_error{"Failed to create Shadow Atlas texture."};
  }

  D3D11_SHADER_RESOURCE_VIEW_DESC constexpr srvDesc{
    .Format = DXGI_FORMAT_R32_FLOAT,
    .ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D,
    .Texture2D = {.MostDetailedMip = 0, .MipLevels = 1}
  };

  if (FAILED(device->CreateShaderResourceView(mTex.Get(), &srvDesc, mSrv.GetAddressOf()))) {
    throw std::runtime_error{"Failed to create Shadow Atlas SRV."};
  }

  D3D11_DEPTH_STENCIL_VIEW_DESC constexpr dsvDesc{
    .Format = DXGI_FORMAT_D32_FLOAT,
    .ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D,
    .Flags = 0,
    .Texture2D = {.MipSlice = 0}
  };

  if (FAILED(device->CreateDepthStencilView(mTex.Get(), &dsvDesc, mDsv.GetAddressOf()))) {
    throw std::runtime_error{"Failed to create Shadow Atlas DSV."};
  }
}


ShadowAtlas::Cell::Cell(int const subdivSize):
  GridLike{subdivSize} {
  mSubcells.resize(GetElementCount());
}


auto ShadowAtlas::Cell::GetSubcell(int const idx) const -> std::optional<Subcell> const& {
  ThrowIfIndexIsInvalid(idx);
  return mSubcells[idx];
}


auto ShadowAtlas::Cell::GetSubcell(int const idx) -> std::optional<Subcell>& {
  return const_cast<std::optional<Subcell>&>(const_cast<Cell const*>(this)->GetSubcell(idx));
}


auto ShadowAtlas::Cell::Resize(int const subdivSize) -> void {
  SetSubdivisionSize(subdivSize);
  mSubcells.resize(GetElementCount());
}


ShadowAtlas::~ShadowAtlas() = default;


auto ShadowAtlas::GetDsv() const noexcept -> ID3D11DepthStencilView* {
  return mDsv.Get();
}


auto ShadowAtlas::GetSrv() const noexcept -> ID3D11ShaderResourceView* {
  return mSrv.Get();
}


auto ShadowAtlas::GetSize() const noexcept -> int {
  return mSize;
}


auto ShadowAtlas::SetLookUpInfo(std::span<ShaderLight> lights) const -> void {
  for (int i = 0; i < GetElementCount(); i++) {
    auto const& cell{GetCell(i)};

    for (int j = 0; j < cell.GetElementCount(); j++) {
      if (auto const& subcell{cell.GetSubcell(j)}) {
        lights[subcell->visibleLightIdxIdx].isCastingShadow = TRUE;
        lights[subcell->visibleLightIdxIdx].sampleShadowMap[subcell->shadowMapIdx] = TRUE;
        lights[subcell->visibleLightIdxIdx].shadowViewProjMatrices[subcell->shadowMapIdx] = subcell->shadowViewProjMtx;
        lights[subcell->visibleLightIdxIdx].shadowAtlasCellOffsets[subcell->shadowMapIdx] = GetNormalizedElementOffset(i) + cell.GetNormalizedElementOffset(j) * GetNormalizedElementSize();
        lights[subcell->visibleLightIdxIdx].shadowAtlasCellSizes[subcell->shadowMapIdx] = GetNormalizedElementSize() * cell.GetNormalizedElementSize();
      }
    }
  }
}
}
