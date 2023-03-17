#pragma once

#include <cstdint>

#include "Math.hpp"
#include "Util.hpp"

namespace leopph {
class RenderCamera {
public:
	enum class Type : std::uint8_t {
		Perspective  = 0,
		Orthographic = 1
	};

	enum class Side : std::uint8_t {
		Vertical   = 0,
		Horizontal = 1
	};

	[[nodiscard]] virtual auto GetPosition() const noexcept -> Vector3 = 0;
	[[nodiscard]] virtual auto GetForwardAxis() const noexcept -> Vector3 = 0;
	[[nodiscard]] virtual auto GetNearClipPlane() const noexcept -> float = 0;
	[[nodiscard]] virtual auto GetFarClipPlane() const noexcept -> float = 0;
	[[nodiscard]] virtual auto GetHorizontalOrthographicSize() const -> float = 0;
	[[nodiscard]] virtual auto GetHorizontalPerspectiveFov() const -> float = 0;
	[[nodiscard]] virtual auto GetType() const -> Type = 0;
	[[nodiscard]] virtual auto GetFrustum(float aspectRatio) const -> Frustum = 0;

	RenderCamera() = default;
	RenderCamera(RenderCamera const& other) = default;
	RenderCamera(RenderCamera&& other) noexcept = default;

	RenderCamera& operator=(RenderCamera const& other) = default;
	RenderCamera& operator=(RenderCamera&& other) noexcept = default;

	virtual ~RenderCamera() = default;
};
}
