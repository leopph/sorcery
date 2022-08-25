#pragma once

#include "Color.hpp"
#include "Event.hpp"
#include "Extent.hpp"
#include "Frustum.hpp"
#include "Math.hpp"
#include "Node.hpp"
#include "Skybox.hpp"
#include "Types.hpp"
#include "Window.hpp"

#include <memory>
#include <variant>


namespace leopph
{
	class CameraNode : public Node, public EventReceiver<WindowEvent>
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
			LEOPPHAPI CameraNode();
			LEOPPHAPI CameraNode(CameraNode const& other);
			LEOPPHAPI CameraNode(CameraNode&& other) noexcept;

		public:
			LEOPPHAPI ~CameraNode() override;

		protected:
			LEOPPHAPI CameraNode& operator=(CameraNode const& other);
			LEOPPHAPI CameraNode& operator=(CameraNode&& other) noexcept;

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


	class OrthographicCameraNode final : public CameraNode
	{
		public:
			[[nodiscard]] LEOPPHAPI f32 get_size(Side side = Side::Horizontal) const;
			LEOPPHAPI void size(f32 size, Side side = Side::Horizontal);

			[[nodiscard]] LEOPPHAPI Matrix4 build_projection_matrix() const override;
			[[nodiscard]] LEOPPHAPI Frustum build_frustum() const override;

		private:
			float mHorizSize{10};
	};


	class PerspectiveCameraNode final : public CameraNode
	{
		public:
			[[nodiscard]] LEOPPHAPI f32 get_fov(Side side = Side::Horizontal) const;
			LEOPPHAPI void set_fov(f32 degrees, Side side = Side::Horizontal);

			[[nodiscard]] LEOPPHAPI Matrix4 build_projection_matrix() const override;
			[[nodiscard]] LEOPPHAPI Frustum build_frustum() const override;

		private:
			[[nodiscard]] f32 convert_fov(f32 fov, bool vert2Horiz) const;

			f32 mHorizFovDeg{100.f};
	};
}
