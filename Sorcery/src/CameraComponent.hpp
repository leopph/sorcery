#pragma once

#include "Component.hpp"
#include "Renderer.hpp"

#include "Math.hpp"
#include "Util.hpp"


namespace sorcery {
class CameraComponent : public Component, public Camera {
  RTTR_ENABLE(Component)
  NormalizedViewport mViewport{ { 0, 0 }, { 1, 1 } };
  Vector4 mBackgroundColor{ 0, 0, 0, 1 };

public:
  LEOPPHAPI CameraComponent();
  ~CameraComponent() override;

  // Viewport extents are normalized between 0 and 1.
  [[nodiscard]] LEOPPHAPI auto GetViewport() const -> NormalizedViewport const&;
  LEOPPHAPI auto SetViewport(NormalizedViewport const& viewport) -> void;

  [[nodiscard]] LEOPPHAPI auto GetBackgroundColor() const -> Vector4 const&;
  LEOPPHAPI auto SetBackgroundColor(Vector4 const& color) -> void;

  [[nodiscard]] auto LEOPPHAPI GetPosition() const noexcept -> Vector3 override;
  [[nodiscard]] auto LEOPPHAPI GetRightAxis() const noexcept -> Vector3 override;
  [[nodiscard]] auto LEOPPHAPI GetUpAxis() const noexcept -> Vector3 override;
  [[nodiscard]] auto LEOPPHAPI GetForwardAxis() const noexcept -> Vector3 override;

  LEOPPHAPI auto OnDrawProperties(bool& changed) -> void override;
};
}
