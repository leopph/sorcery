#pragma once

#include "Camera.hpp"


namespace leopph
{
	class OrthographicCamera final : public Camera
	{
		public:
			[[nodiscard]] LEOPPHAPI f32 get_size(Side side = Side::Horizontal) const;
			LEOPPHAPI void size(f32 size, Side side = Side::Horizontal);

			[[nodiscard]] LEOPPHAPI Matrix4 build_projection_matrix() const override;
			[[nodiscard]] LEOPPHAPI leopph::Frustum build_frustum() const override;


			OrthographicCamera() = default;

			OrthographicCamera(OrthographicCamera const& other) = delete;
			OrthographicCamera& operator=(OrthographicCamera const& other) = delete;

			OrthographicCamera(OrthographicCamera&& other) noexcept = delete;
			OrthographicCamera& operator=(OrthographicCamera&& other) noexcept = delete;

			~OrthographicCamera() override = default;


		private:
			float mHorizSize{10};
	};
}
