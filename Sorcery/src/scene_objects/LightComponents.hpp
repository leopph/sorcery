#pragma once

#include "Component.hpp"
#include "../Math.hpp"

#include <array>


namespace sorcery {
class LightComponent final : public Component {
  RTTR_ENABLE(Component)

public:
  [[nodiscard]] LEOPPHAPI auto Clone() -> LightComponent* override;

  [[nodiscard]] LEOPPHAPI auto GetColor() const -> Vector3 const&;
  LEOPPHAPI auto SetColor(Vector3 const& color) -> void;

  [[nodiscard]] LEOPPHAPI auto GetIntensity() const -> f32;
  LEOPPHAPI auto SetIntensity(f32 intensity) -> void;

  [[nodiscard]] LEOPPHAPI auto IsCastingShadow() const -> bool;
  LEOPPHAPI auto SetCastingShadow(bool castShadow) -> void;


  enum class Type {
    Directional = 0,
    Spot        = 1,
    Point       = 2
  };


  [[nodiscard]] LEOPPHAPI auto GetType() const noexcept -> Type;
  LEOPPHAPI auto SetType(Type type) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetDirection() const -> Vector3 const&;

  [[nodiscard]] LEOPPHAPI auto GetShadowNearPlane() const -> f32;
  LEOPPHAPI auto SetShadowNearPlane(f32 nearPlane) -> void;

  [[nodiscard]] LEOPPHAPI auto GetRange() const -> f32;
  LEOPPHAPI auto SetRange(f32 range) -> void;

  [[nodiscard]] LEOPPHAPI auto GetInnerAngle() const -> f32;
  LEOPPHAPI auto SetInnerAngle(f32 degrees) -> void;

  [[nodiscard]] LEOPPHAPI auto GetOuterAngle() const -> f32;
  LEOPPHAPI auto SetOuterAngle(f32 degrees) -> void;

  [[nodiscard]] LEOPPHAPI auto GetShadowNormalBias() const noexcept -> float;
  LEOPPHAPI auto SetShadowNormalBias(float bias) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetShadowDepthBias() const noexcept -> float;
  LEOPPHAPI auto SetShadowDepthBias(float bias) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetShadowExtension() const noexcept -> float;
  LEOPPHAPI auto SetShadowExtension(float shadowExtension) noexcept -> void;

  LEOPPHAPI auto OnInit() -> void override;
  LEOPPHAPI auto OnDestroy() -> void override;
  LEOPPHAPI auto OnDrawProperties(bool& changed) -> void override;
  LEOPPHAPI auto OnDrawGizmosSelected() -> void override;

  constexpr static auto MIN_INTENSITY{0.0f};
  constexpr static auto MIN_SHADOW_NEAR_PLANE{0.1f};
  constexpr static auto MIN_RANGE{0.0f};
  constexpr static auto MIN_ANGLE_DEG{0.0f};
  constexpr static auto MAX_ANGLE_DEG{179.0f};
  constexpr static auto MIN_SHADOW_EXTENSION{0.0f};

private:
  bool mCastsShadow{false};
  Vector3 mColor{1.f};
  f32 mIntensity{1.f};
  Type mType{Type::Directional};
  f32 mShadowNear{MIN_SHADOW_NEAR_PLANE};
  f32 mRange{10.f};
  f32 mInnerAngle{30.f};
  f32 mOuterAngle{30.f};
  float mShadowNormalBias{0.0f};
  float mShadowDepthBias{0.0f};
  float mShadowExtension{MIN_SHADOW_EXTENSION};
};


[[nodiscard]] LEOPPHAPI auto CalculateSpotLightLocalVertices(float range,
                                                             float outer_angle) noexcept -> std::array<Vector3, 5>;
}
