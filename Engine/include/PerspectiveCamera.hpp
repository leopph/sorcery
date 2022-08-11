#pragma once

#include "Camera.hpp"
#include "Types.hpp"


namespace leopph
{
	// Special Camera using a perspective projection.
	class PerspectiveCamera final : public Camera
	{
		public:
			[[nodiscard]] LEOPPHAPI f32 get_fov(Side side = Side::Horizontal) const;
			LEOPPHAPI void set_fov(f32 degrees, Side side = Side::Horizontal);


			[[nodiscard]] LEOPPHAPI Matrix4 build_projection_matrix() const override;
			[[nodiscard]] LEOPPHAPI leopph::Frustum build_frustum() const override;


			PerspectiveCamera() = default;

			PerspectiveCamera(PerspectiveCamera const& other) = delete;
			PerspectiveCamera& operator=(PerspectiveCamera const& other) = delete;

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
			f32 convert_fov(f32 fov, Conversion conversion) const;

			f32 mHorizFovDeg{100.f};
	};
}
