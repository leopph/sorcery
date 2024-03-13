#pragma once

#include "../Core.hpp"
#include "../Math.hpp"
#include "render_target.hpp"

#include <cstdint>


namespace sorcery::rendering {
class Camera {
public:
  enum class Type : std::uint8_t {
    Perspective  = 0,
    Orthographic = 1
  };

private:
  constexpr static float MINIMUM_PERSPECTIVE_NEAR_CLIP_PLANE{0.01f};
  constexpr static float MINIMUM_PERSPECTIVE_FAR_CLIP_PLANE_OFFSET{0.1f};
  constexpr static float MINIMUM_PERSPECTIVE_VERTICAL_FOV{5.0f};
  constexpr static float MINIMUM_ORTHOGRAPHIC_VERTICAL_SIZE{0.1f};

  float mNear{MINIMUM_PERSPECTIVE_NEAR_CLIP_PLANE};
  float mFar{100.f};
  float mVertOrhoSize{10};
  float mVertPerspFovDeg{60};
  Type mType{Type::Perspective};
  std::shared_ptr<RenderTarget> render_target_{nullptr};

public:
  [[nodiscard]] virtual auto GetPosition() const noexcept -> Vector3 = 0;
  [[nodiscard]] virtual auto GetRightAxis() const noexcept -> Vector3 = 0;
  [[nodiscard]] virtual auto GetUpAxis() const noexcept -> Vector3 = 0;
  [[nodiscard]] virtual auto GetForwardAxis() const noexcept -> Vector3 = 0;

  [[nodiscard]] LEOPPHAPI auto GetNearClipPlane() const noexcept -> float;
  LEOPPHAPI auto SetNearClipPlane(float near_clip_plane) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetFarClipPlane() const noexcept -> float;
  LEOPPHAPI auto SetFarClipPlane(float far_clip_plane) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetType() const noexcept -> Type;
  LEOPPHAPI auto SetType(Type type) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetVerticalPerspectiveFov() const -> float;
  LEOPPHAPI auto SetVerticalPerspectiveFov(float degrees) -> void;

  [[nodiscard]] LEOPPHAPI auto GetVerticalOrthographicSize() const -> float;
  LEOPPHAPI auto SetVerticalOrthographicSize(float size) -> void;

  [[nodiscard]] LEOPPHAPI auto GetRenderTarget() const -> std::shared_ptr<RenderTarget> const&;
  LEOPPHAPI auto SetRenderTarget(std::shared_ptr<RenderTarget> rt) -> void;

  [[nodiscard]] LEOPPHAPI auto CalculateViewMatrix() const noexcept -> Matrix4;
  [[nodiscard]] LEOPPHAPI auto CalculateProjectionMatrix(float aspect_ratio) const noexcept -> Matrix4;

  [[nodiscard]] LEOPPHAPI static auto HorizontalPerspectiveFovToVertical(
    float fov_degrees, float aspect_ratio) noexcept -> float;
  [[nodiscard]] LEOPPHAPI static auto VerticalPerspectiveFovToHorizontal(
    float fov_degrees, float aspect_ratio) noexcept -> float;

  [[nodiscard]] LEOPPHAPI static auto CalculateViewMatrix(Vector3 const& position, Vector3 const& right,
                                                          Vector3 const& up, Vector3 const& forward) -> Matrix4;
  [[nodiscard]] LEOPPHAPI static auto CalculateProjectionMatrix(Type type, float fov_deg_vert, float size_vert,
                                                                float aspect_ratio, float near_plane,
                                                                float far_plane) -> Matrix4;

  Camera() = default;
  Camera(Camera const& other) = default;
  Camera(Camera&& other) noexcept = default;

  auto operator=(Camera const& other) -> Camera& = default;
  auto operator=(Camera&& other) noexcept -> Camera& = default;

  virtual ~Camera() = default;
};
}
