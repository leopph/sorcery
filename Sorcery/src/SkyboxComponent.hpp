#pragma once

#include "Component.hpp"
#include "Resources/Cubemap.hpp"


namespace sorcery {
class SkyboxComponent : public Component {
  RTTR_ENABLE(Component)
  Cubemap* mCubemap{ nullptr };

public:
  Type constexpr static SerializationType{ Type::Skybox };
  [[nodiscard]] auto LEOPPHAPI GetSerializationType() const -> Type override;

  [[nodiscard]] auto LEOPPHAPI GetCubemap() const noexcept -> Cubemap*;
  auto LEOPPHAPI SetCubemap(Cubemap* cubemap) noexcept -> void;

  SkyboxComponent() = default;
  SkyboxComponent(SkyboxComponent const& other) = delete;
  SkyboxComponent(SkyboxComponent&& other) = delete;

  SkyboxComponent& operator=(SkyboxComponent const& other) = delete;
  SkyboxComponent& operator=(SkyboxComponent&& other) = delete;

  LEOPPHAPI ~SkyboxComponent() override;
};
}
