#pragma once

#include "Color.hpp"
#include "Component.hpp"
#include "EventReceiver.hpp"
#include "Extent.hpp"
#include "Frustum.hpp"
#include "Math.hpp"
#include "Skybox.hpp"
#include "Types.hpp"
#include "Window.hpp"

#include <memory>
#include <variant>


namespace leopph
{
	class Camera : public Component, public EventReceiver<WindowEvent>
	{
		public:
			enum class Side
			{
				Vertical,
				Horizontal
			};



			[[nodiscard]] LEOPPHAPI f32 get_near_clip_plane() const;
			LEOPPHAPI void set_near_clip_plane(f32 near);


			[[nodiscard]] LEOPPHAPI f32 get_far_clip_plane() const;
			LEOPPHAPI void set_far_clip_plane(f32 far);


			[[nodiscard]] LEOPPHAPI std::variant<Color, std::shared_ptr<Skybox>> const& get_background() const;
			LEOPPHAPI void set_background(std::variant<Color, std::shared_ptr<Skybox>> background);

			// Viewport extents are normalized between 0 and 1.
			[[nodiscard]] LEOPPHAPI Extent<f32> const& get_viewport() const;
			LEOPPHAPI void set_viewport(Extent<f32> const& viewport);


			[[nodiscard]] LEOPPHAPI Extent<u32> get_window_extents() const;
			LEOPPHAPI void set_window_extents(Extent<u32> const& extent);


			[[nodiscard]] LEOPPHAPI f32 get_aspect_ratio() const;


			LEOPPHAPI void enable();
			LEOPPHAPI void disable();
			[[nodiscard]] bool is_enabled() const;


			[[nodiscard]] LEOPPHAPI Matrix4 build_view_matrix() const;
			[[nodiscard]] virtual Matrix4 build_projection_matrix() const = 0;


			[[nodiscard]] virtual Frustum build_frustum() const = 0;


			// Transforms the passed vector (interpreted as a point) to viewport space.
			// Viewport space ranges from [0, 1] on both axes. Values outside of that are not visible.
			[[nodiscard]] LEOPPHAPI Vector2 transform_to_viewport(Vector3 const& vector) const;


		protected:
			LEOPPHAPI Camera();

		public:
			Camera(Camera const&) = delete;
			Camera(Camera&&) = delete;

			Camera& operator=(Camera const&) = delete;
			void operator=(Camera&&) = delete;

			LEOPPHAPI ~Camera() override;


		private:
			LEOPPHAPI void on_event(WindowEvent const& event) override;

			std::variant<Color, std::shared_ptr<Skybox>> mBackground{Color{0, 0, 0, 255}};
			f32 mNear{0.1f};
			f32 mFar{100.f};
			Extent<f32> mViewport{0, 0, 1, 1};
			Extent<u32> mWindowExtent;
			f32 mAspectRatio;
			bool mEnabled{true};
	};


	class OrthographicCamera final : public Camera
	{
		public:
			[[nodiscard]] LEOPPHAPI f32 get_size(Side side = Side::Horizontal) const;
			LEOPPHAPI void size(f32 size, Side side = Side::Horizontal);

			[[nodiscard]] LEOPPHAPI Matrix4 build_projection_matrix() const override;
			[[nodiscard]] LEOPPHAPI Frustum build_frustum() const override;


			OrthographicCamera() = default;

			OrthographicCamera(OrthographicCamera const& other) = delete;
			OrthographicCamera& operator=(OrthographicCamera const& other) = delete;

			OrthographicCamera(OrthographicCamera&& other) noexcept = delete;
			OrthographicCamera& operator=(OrthographicCamera&& other) noexcept = delete;

			~OrthographicCamera() override = default;


		private:
			float mHorizSize{10};
	};


	class PerspectiveCamera final : public Camera
	{
		public:
			[[nodiscard]] LEOPPHAPI f32 get_fov(Side side = Side::Horizontal) const;
			LEOPPHAPI void set_fov(f32 degrees, Side side = Side::Horizontal);


			[[nodiscard]] LEOPPHAPI Matrix4 build_projection_matrix() const override;
			[[nodiscard]] LEOPPHAPI Frustum build_frustum() const override;


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
