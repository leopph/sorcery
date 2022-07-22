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
			[[nodiscard]] LEOPPHAPI float Fov(Side side = Side::Horizontal) const noexcept;

			// Set the current field of view of the Camera in degrees.
			// side specifies whether the value is interpreted horizontally or vertically.
			LEOPPHAPI void Fov(float degrees, Side side = Side::Horizontal) noexcept;

			[[nodiscard]] LEOPPHAPI Matrix4 ProjectionMatrix() const override;

			[[nodiscard]] LEOPPHAPI leopph::Frustum Frustum() const override;

			PerspectiveCamera() = default;

			PerspectiveCamera(PerspectiveCamera const& other) = default;
			PerspectiveCamera& operator=(PerspectiveCamera const& other) = default;

			[[nodiscard]] LEOPPHAPI ComponentPtr<> Clone() const override;

			PerspectiveCamera(PerspectiveCamera&& other) noexcept = delete;
			PerspectiveCamera& operator=(PerspectiveCamera&& other) noexcept = delete;

			~PerspectiveCamera() override = default;

		private:
			enum class Conversion
			{
				VerticalToHorizontal,
				HorizontalToVertical
			};


			[[nodiscard]]
			float ConvertFov(float fov, Conversion conversion) const;

			float m_HorizontalFovDegrees{100.f};
	};
}
