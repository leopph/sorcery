#pragma once

#include "Camera.hpp"
#include "ShadowCascadeBoundary.hpp"
#include "../GridLike.hpp"

#include <optional>
#include <span>
#include <vector>


namespace sorcery {
class ShadowAtlas : public GridLike {
protected:
  Microsoft::WRL::ComPtr<ID3D11Texture2D> mTex;
  Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mSrv;
  Microsoft::WRL::ComPtr<ID3D11DepthStencilView> mDsv;
  int mSize;


  ShadowAtlas(ID3D11Device* device, int size, int subdivSize);

public:
  class Cell : public GridLike {
  public:
    struct Subcell {
      Matrix4 shadowViewProjMtx;
      // Index into the array of indices to the visible lights, use lights[visibleLights[visibleLightIdxIdx]] to get to the light
      int visibleLightIdxIdx;
      int shadowMapIdx;
      char pad[sizeof(Matrix4) - 2 * sizeof(int)];
    };

  private:
    std::vector<std::optional<Subcell>> mSubcells;

  public:
    explicit Cell(int subdivSize);

    [[nodiscard]] auto GetSubcell(int idx) const -> std::optional<Subcell> const&;
    [[nodiscard]] auto GetSubcell(int idx) -> std::optional<Subcell>&;
    auto Resize(int subdivSize) -> void;
  };


  ShadowAtlas(ShadowAtlas const&) = delete;
  ShadowAtlas(ShadowAtlas&&) = delete;

  virtual ~ShadowAtlas();

  auto operator=(ShadowAtlas const&) -> void = delete;
  auto operator=(ShadowAtlas&&) -> void = delete;

  [[nodiscard]] auto GetDsv() const noexcept -> ID3D11DepthStencilView*;
  [[nodiscard]] auto GetSrv() const noexcept -> ID3D11ShaderResourceView*;
  [[nodiscard]] auto GetSize() const noexcept -> int;

  auto SetLookUpInfo(std::span<ShaderLight> lights) const -> void;

  [[nodiscard]] virtual auto GetCell(int idx) const -> Cell const& = 0;
};
}
