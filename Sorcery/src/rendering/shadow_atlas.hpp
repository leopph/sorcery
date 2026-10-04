#pragma once

#include <optional>
#include <span>
#include <vector>

#include "ShadowCascadeBoundary.hpp"
#include "../GridLike.hpp"
#include "wand/wand.hpp"


namespace sorcery::rendering {
class ShadowAtlas : public GridLike {
protected:
  wand::SharedDeviceChildHandle<wand::Texture> tex_;
  UINT size_;


  ShadowAtlas(wand::GraphicsDevice* device, DXGI_FORMAT depth_format, UINT size, int subdiv_size);

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
    std::vector<std::optional<Subcell>> subcells_;

  public:
    explicit Cell(int subdiv_size);

    [[nodiscard]] auto GetSubcell(int idx) const -> std::optional<Subcell> const&;
    [[nodiscard]] auto GetSubcell(int idx) -> std::optional<Subcell>&;
    auto Resize(int subdiv_size) -> void;
  };


  ShadowAtlas(ShadowAtlas const&) = delete;
  ShadowAtlas(ShadowAtlas&&) = delete;

  virtual ~ShadowAtlas();

  auto operator=(ShadowAtlas const&) -> void = delete;
  auto operator=(ShadowAtlas&&) -> void = delete;

  [[nodiscard]] auto GetTex() const noexcept -> wand::SharedDeviceChildHandle<wand::Texture> const&;
  [[nodiscard]] auto GetSize() const noexcept -> UINT;

  [[nodiscard]] virtual auto GetCell(int idx) const -> Cell const& = 0;
  [[nodiscard]] virtual auto GetCell(int idx) -> Cell& = 0;
};
}
