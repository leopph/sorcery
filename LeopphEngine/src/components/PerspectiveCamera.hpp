#pragma once

#include "Camera.hpp"


namespace leopph
{
	// Special Camera using a perspective projection.
	class PerspectiveCamera final : public Camera
	{
		public:
			// When adjusting FOV, this is used to specify which direction
			// the FOV value should be interpreted in for non-symmetric viewing volumes.
			enum class FovDirection
			{
				Horizontal,
				Vertical
			};


			PerspectiveCamera() = default;

			// Set the current FOV value of the Camera in degrees.
			// For non-symmetric viewing volumes, direction specifies the interpretation.
			LEOPPHAPI auto Fov(float degrees, FovDirection direction) -> void;

			// Get the current FOV value of the Camera in degrees.
			// For non-symmetric viewing volumes, direction specifies the interpretation.
			LEOPPHAPI auto Fov(FovDirection direction) const -> float;

			[[nodiscard]] LEOPPHAPI
			auto ProjectionMatrix() const -> Matrix4 override;

			[[nodiscard]] LEOPPHAPI
			auto Frustum() const -> leopph::Frustum override;

			PerspectiveCamera(PerspectiveCamera const& other) = delete;
			auto operator=(PerspectiveCamera const& other) -> PerspectiveCamera& = delete;

			PerspectiveCamera(PerspectiveCamera&& other) noexcept = delete;
			auto operator=(PerspectiveCamera&& other) noexcept -> PerspectiveCamera& = delete;

			~PerspectiveCamera() override = default;

		private:
			enum class FovConversionDirection
			{
				VerticalToHorizontal,
				HorizontalToVertical
			};


			[[nodiscard]]
			auto ConvertFov(float fov, FovConversionDirection conversion) const -> float;

			float m_HorizontalFovDegrees{100.f};
	};
}
