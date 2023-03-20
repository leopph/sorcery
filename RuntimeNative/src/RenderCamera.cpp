#include "RenderCamera.hpp"

namespace leopph {
auto RenderCamera::HorizontalPerspectiveFovToVertical(float const fovDegrees, float const aspectRatio) noexcept -> float {
	return ToDegrees(2.0f * std::atan(std::tan(ToRadians(fovDegrees) / 2.0f) / aspectRatio));
}


auto RenderCamera::VerticalPerspectiveFovToHorizontal(float const fovDegrees, float const aspectRatio) noexcept -> float {
	return ToDegrees(2.0f * std::atan(std::tan(ToRadians(fovDegrees) / 2.0f) * aspectRatio));
}
}
