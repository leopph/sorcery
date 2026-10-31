#pragma once

#include <sorcery/Math.hpp>
#include <sorcery/rendering/Camera.hpp>
#include <sorcery/scene/Component.hpp>


namespace sorcery {
class CameraComponent final : public Component, public rendering::Camera {
  RTTR_ENABLE(Component)

public:
  [[nodiscard]] LEOPPHAPI auto Clone() -> std::unique_ptr<SceneObject> override;
  LEOPPHAPI auto OnAfterEnteringWorld(World& world) -> void override;
  LEOPPHAPI auto OnBeforeExitingWorld(World& world) -> void override;

  [[nodiscard]] LEOPPHAPI auto GetBackgroundColor() const -> Vector4 const&;
  LEOPPHAPI auto SetBackgroundColor(Vector4 const& color) -> void;

  // Cameras with lower order render first; higher order cameras draw on top of them.
  [[nodiscard]] SORCERYAPI auto GetOrder() const noexcept -> int;
  SORCERYAPI auto SetOrder(int order) noexcept -> void;

  [[nodiscard]] auto LEOPPHAPI GetPosition() const noexcept -> Vector3 override;
  [[nodiscard]] auto LEOPPHAPI GetRightAxis() const noexcept -> Vector3 override;
  [[nodiscard]] auto LEOPPHAPI GetUpAxis() const noexcept -> Vector3 override;
  [[nodiscard]] auto LEOPPHAPI GetForwardAxis() const noexcept -> Vector3 override;

private:
  Vector4 background_color_{0, 0, 0, 1};
  int order_{0};
};
}
