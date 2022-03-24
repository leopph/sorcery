#pragma once

#include "Camera.hpp"


namespace leopph
{
	class OrthographicCamera final : public Camera
	{
		public:
			// Get the size of the camera in engine units.
			// side specifies whether the value should be interpreted horizontally or vertically.
			[[nodiscard]] LEOPPHAPI
			auto Size(Side side = Side::Horizontal) const noexcept -> float;

			// Set the size of the camera in engine units.
			// side specifies whether the value is interpreted horizontally or vertically.
			LEOPPHAPI
			auto Size(float size, Side side = Side::Horizontal) noexcept -> void;

			[[nodiscard]] LEOPPHAPI
			auto ProjectionMatrix() const -> Matrix4 override;

			[[nodiscard]] LEOPPHAPI
			auto Frustum() const -> leopph::Frustum override;

			OrthographicCamera() = default;

			OrthographicCamera(OrthographicCamera const& other) = delete;
			auto operator=(OrthographicCamera const& other) -> OrthographicCamera& = delete;

			OrthographicCamera(OrthographicCamera&& other) noexcept = delete;
			auto operator=(OrthographicCamera&& other) noexcept -> OrthographicCamera& = delete;

			~OrthographicCamera() override = default;

		private:
			float m_HorizontalSize{10};
	};
}
