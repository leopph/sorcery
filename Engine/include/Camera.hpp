#pragma once

#include "Color.hpp"
#include "Component.hpp"
#include "EventReceiver.hpp"
#include "Frustum.hpp"
#include "Matrix.hpp"
#include "RenderingPath.hpp"
#include "Skybox.hpp"
#include "Types.hpp"
#include "WindowEvent.hpp"

#include <memory>
#include <variant>


namespace leopph
{
	class Camera : public Component, public EventReceiver<internal::WindowEvent>
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


			[[nodiscard]] LEOPPHAPI Matrix4 build_view_matrix() const;
			[[nodiscard]] virtual Matrix4 build_projection_matrix() const = 0;


			[[nodiscard]] virtual Frustum build_frustum() const = 0;


			// Transforms the passed vector (interpreted as a point) to viewport space.
			// Viewport space ranges from [0, 1] on both axes. Values outside of that are not visible.
			[[nodiscard]] LEOPPHAPI Vector2 transform_to_viewport(Vector3 const& vector) const;


			[[nodiscard]] LEOPPHAPI RenderingPath get_rendering_path() const;
			LEOPPHAPI void set_rendering_path(RenderingPath path);


		protected:
			[[nodiscard]] f32 get_aspect_ratio() const;

			LEOPPHAPI Camera();


		public:
			Camera(Camera const&) = delete;
			Camera& operator=(Camera const&) = delete;

			Camera(Camera&&) = delete;
			void operator=(Camera&&) = delete;

			LEOPPHAPI ~Camera() override;


		private:
			LEOPPHAPI void OnEventReceived(EventParamType event) override;

			float mAspectRatio;
			float mNear{0.1f};
			float mFar{100.f};
			std::variant<Color, std::shared_ptr<Skybox>> mBackground{Color{0, 0, 0, 255}};
			RenderingPath mRenderingPath{RenderingPath::Forward};
	};
}
