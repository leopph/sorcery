#pragma once

#include <Renderer.hpp>

namespace leopph::editor {
struct EditorCamera : renderer::Camera {
	[[nodiscard]] auto GetPosition() const noexcept -> Vector3 override;
	[[nodiscard]] auto GetForwardAxis() const noexcept -> Vector3 override;
	[[nodiscard]] auto GetNearClipPlane() const noexcept -> float override;
	[[nodiscard]] auto GetFarClipPlane() const noexcept -> float override;
	[[nodiscard]] auto GetHorizontalOrthographicSize() const -> float override;
	[[nodiscard]] auto GetHorizontalPerspectiveFov() const -> float override;
	[[nodiscard]] auto GetType() const -> Type override;

	EditorCamera(Vector3 const& position, Quaternion const& orientation, float nearClip, float farClip, float fovHorizDeg);
	[[nodiscard]] auto GetFrustum(float aspectRatio) const -> Frustum override;

	Vector3 position;
	Quaternion orientation;
	float nearClip;
	float farClip;
	float fovHorizDeg;
};
}
