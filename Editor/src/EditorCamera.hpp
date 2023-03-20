#pragma once

#include <Renderer.hpp>

namespace leopph::editor {
struct EditorCamera : renderer::Camera {
	[[nodiscard]] auto GetPosition() const noexcept -> Vector3 override;
	[[nodiscard]] auto GetForwardAxis() const noexcept -> Vector3 override;

	EditorCamera(Vector3 const& position, Quaternion const& orientation, float nearClip, float farClip, float fovHorizDeg);

	Vector3 position;
	Quaternion orientation;
};
}
