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

	[[nodiscard]] static auto LEOPPHAPI HorizontalPerspectiveFovToVertical(float fovDegrees, float aspectRatio) noexcept -> float;
	[[nodiscard]] static auto LEOPPHAPI VerticalPerspectiveFovToHorizontal(float fovDegrees, float aspectRatio) noexcept -> float;

	RenderCamera() = default;
	RenderCamera(RenderCamera const& other) = default;
	RenderCamera(RenderCamera&& other) noexcept = default;

	auto operator=(RenderCamera const& other) -> RenderCamera& = default;
	auto operator=(RenderCamera&& other) noexcept -> RenderCamera& = default;

	virtual ~RenderCamera() = default;
};
}
