#pragma once

#include "Component.hpp"
#include "../Resources/Cubemap.hpp"


namespace sorcery {
class SkyboxComponent : public Component {
  RTTR_ENABLE(Component)
  Cubemap* mCubemap{nullptr};

public:
  [[nodiscard]] auto LEOPPHAPI GetCubemap() const noexcept -> Cubemap*;
  auto LEOPPHAPI SetCubemap(Cubemap* cubemap) noexcept -> void;

  LEOPPHAPI auto OnDestroy() -> void override;
  LEOPPHAPI auto OnDrawProperties(bool& changed) -> void override;
};
}
