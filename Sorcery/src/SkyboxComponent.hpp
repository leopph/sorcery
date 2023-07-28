#pragma once

#include "Component.hpp"
#include "Resources/Cubemap.hpp"


namespace sorcery {
class SkyboxComponent : public Component {
  RTTR_ENABLE(Component)
  Cubemap* mCubemap{ nullptr };

public:
  [[nodiscard]] auto LEOPPHAPI GetCubemap() const noexcept -> Cubemap*;
  auto LEOPPHAPI SetCubemap(Cubemap* cubemap) noexcept -> void;

  SkyboxComponent() = default;
  SkyboxComponent(SkyboxComponent const& other) = delete;
  SkyboxComponent(SkyboxComponent&& other) = delete;

  auto operator=(SkyboxComponent const& other) -> SkyboxComponent& = delete;
  auto operator=(SkyboxComponent&& other) -> SkyboxComponent& = delete;

  LEOPPHAPI ~SkyboxComponent() override;

  LEOPPHAPI auto OnDrawProperties() -> void override;
};
}
