#pragma once

#include "Camera.hpp"


namespace leopph
{
	// Special Camera using a perspective projection.
	class PerspectiveCamera final : public Camera
	{
		public:
			// Get the current field of view of the Camera in degrees.
			// side specifies whether the value should be interpreted horizontally or vertically.
			[[nodiscard]] LEOPPHAPI
			auto Fov(Side side = Side::Horizontal) const noexcept -> float;

			// Set the current field of view of the Camera in degrees.
			// side specifies whether the value is interpreted horizontally or vertically.
			LEOPPHAPI
			auto Fov(float degrees, Side side = Side::Horizontal) noexcept -> void;

			[[nodiscard]] LEOPPHAPI
			auto ProjectionMatrix() const -> Matrix4 override;

			[[nodiscard]] LEOPPHAPI
			auto Frustum() const -> leopph::Frustum override;

			PerspectiveCamera() = default;

			PerspectiveCamera(PerspectiveCamera const& other) = default;
			auto operator=(PerspectiveCamera const& other) -> PerspectiveCamera& = default;

			[[nodiscard]] LEOPPHAPI
			auto Clone() const -> ComponentPtr<> override;

			PerspectiveCamera(PerspectiveCamera&& other) noexcept = delete;
			auto operator=(PerspectiveCamera&& other) noexcept -> PerspectiveCamera& = delete;

			~PerspectiveCamera() override = default;

		private:
			enum class Conversion
			{
				VerticalToHorizontal,
				HorizontalToVertical
			};


			[[nodiscard]]
			auto ConvertFov(float fov, Conversion conversion) const -> float;

			float m_HorizontalFovDegrees{100.f};
	};
}
