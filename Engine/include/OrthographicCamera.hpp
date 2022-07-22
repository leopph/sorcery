#pragma once

#include "Camera.hpp"


namespace leopph
{
	class OrthographicCamera final : public Camera
	{
		public:
			// Get the size of the camera in engine units.
			// side specifies whether the value should be interpreted horizontally or vertically.
			[[nodiscard]] LEOPPHAPI float Size(Side side = Side::Horizontal) const noexcept;

			// Set the size of the camera in engine units.
			// side specifies whether the value is interpreted horizontally or vertically.
			LEOPPHAPI void Size(float size, Side side = Side::Horizontal) noexcept;

			[[nodiscard]] LEOPPHAPI Matrix4 ProjectionMatrix() const override;

			[[nodiscard]] LEOPPHAPI leopph::Frustum Frustum() const override;

			OrthographicCamera() = default;

			OrthographicCamera(OrthographicCamera const& other) = default;
			OrthographicCamera& operator=(OrthographicCamera const& other) = default;

			[[nodiscard]] LEOPPHAPI ComponentPtr<> Clone() const override;

			OrthographicCamera(OrthographicCamera&& other) noexcept = delete;
			OrthographicCamera& operator=(OrthographicCamera&& other) noexcept = delete;

			~OrthographicCamera() override = default;

		private:
			float m_HorizontalSize{10};
	};
}
