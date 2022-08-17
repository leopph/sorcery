#include "Camera.hpp"

#include "Context.hpp"
#include "Entity.hpp"
#include "Window.hpp"
#include "WindowEvent.hpp"
#include "../rendering/Renderer.hpp"

#include <algorithm>
#include <utility>


namespace leopph
{
	f32 Camera::get_near_clip_plane() const
	{
		return mNear;
	}



	void Camera::set_near_clip_plane(f32 const near)
	{
		mNear = near;
	}



	f32 Camera::get_far_clip_plane() const
	{
		return mFar;
	}



	void Camera::set_far_clip_plane(f32 const far)
	{
		mFar = far;
	}



	std::variant<Color, std::shared_ptr<Skybox>> const& Camera::get_background() const
	{
		return mBackground;
	}



	void Camera::set_background(std::variant<Color, std::shared_ptr<Skybox>> background)
	{
		mBackground = std::move(background);
	}



	Extent<f32> const& Camera::get_viewport() const
	{
		return mViewport;
	}



	void Camera::set_viewport(Extent<f32> const& viewport)
	{
		mViewport = viewport;

		auto const* const window = get_window();
		auto const windowWidth = static_cast<f32>(window->get_width());
		auto const windowHeight = static_cast<f32>(window->get_height());

		mWindowExtent =
		{
			.offsetX = static_cast<u32>(mViewport.offsetX * windowWidth),
			.offsetY = static_cast<u32>(mViewport.offsetY * windowHeight),
			.width = static_cast<u32>(mViewport.width * windowWidth),
			.height = static_cast<u32>(mViewport.height * windowHeight)
		};

		mAspectRatio = static_cast<f32>(mWindowExtent.width) / static_cast<f32>(mWindowExtent.height);
	}



	Extent<u32> Camera::get_window_extents() const
	{
		return mWindowExtent;
	}



	void Camera::set_window_extents(Extent<u32> const& extent)
	{
		auto const* const window = get_window();

		mWindowExtent.offsetX = std::clamp<u32>(extent.offsetX, 0, window->get_width());
		mWindowExtent.offsetY = std::clamp<u32>(extent.offsetY, 0, window->get_height());
		mWindowExtent.width = mWindowExtent.offsetX + extent.width > window->get_width() ? 0 : extent.width;
		mWindowExtent.height = mWindowExtent.offsetY + extent.height > window->get_height() ? 0 : extent.height;

		auto const extentWidth = static_cast<f32>(mWindowExtent.width);
		auto const extentHeight = static_cast<f32>(mWindowExtent.height);

		mAspectRatio = extentWidth / extentHeight;

		auto const windowWidth = static_cast<f32>(window->get_width());
		auto const windowHeight = static_cast<f32>(window->get_height());

		mViewport.offsetX = windowWidth / static_cast<f32>(mWindowExtent.offsetX);
		mViewport.offsetY = windowHeight / static_cast<f32>(mWindowExtent.offsetY);
		mViewport.width = windowWidth / extentWidth;
		mViewport.height = windowHeight / extentHeight;
	}



	f32 Camera::get_aspect_ratio() const
	{
		return mAspectRatio;
	}



	void Camera::enable()
	{
		if (!mEnabled)
		{
			internal::get_renderer()->register_camera(this);
			mEnabled = true;
		}
	}



	void Camera::disable()
	{
		if (mEnabled)
		{
			internal::get_renderer()->unregister_camera(this);
			mEnabled = false;
		}
	}



	bool Camera::is_enabled() const
	{
		return mEnabled;
	}



	Matrix4 Camera::build_view_matrix() const
	{
		// inv(T) * inv(R)
		return Matrix4::translate(-get_owner()->get_position()) * Matrix4{get_owner()->get_rotation()}.transpose();
	}



	Vector2 Camera::transform_to_viewport(Vector3 const& vector) const
	{
		Vector4 homoPos{vector, 1};
		homoPos *= build_view_matrix() * build_projection_matrix();
		Vector2 ret{homoPos};
		ret /= homoPos[3];
		ret *= 0.5f;
		ret += Vector2{0.5f};
		return ret;
	}



	RenderingPath Camera::get_rendering_path() const
	{
		return mRenderingPath;
	}



	void Camera::set_rendering_path(RenderingPath const path)
	{
		mRenderingPath = path;
	}



	Camera::Camera() :
		mWindowExtent
		{
			[this]
			{
				auto const* const window = get_window();
				auto const windowWidth = static_cast<f32>(window->get_width());
				auto const windowHeight = static_cast<f32>(window->get_height());

				return Extent<u32>
				{
					.offsetX = static_cast<u32>(mViewport.offsetX * windowWidth),
					.offsetY = static_cast<u32>(mViewport.offsetY * windowHeight),
					.width = static_cast<u32>(mViewport.width * windowWidth),
					.height = static_cast<u32>(mViewport.height * windowHeight)
				};
			}()
		},
		mAspectRatio{static_cast<f32>(mWindowExtent.width) / static_cast<f32>(mWindowExtent.height)}
	{
		internal::get_renderer()->register_camera(this);
	}



	Camera::~Camera()
	{
		internal::get_renderer()->unregister_camera(this);
	}



	void Camera::on_event(internal::WindowEvent const& event)
	{
		mWindowExtent =
		{
			.offsetX = static_cast<u32>(mViewport.offsetX * static_cast<f32>(event.width)),
			.offsetY = static_cast<u32>(mViewport.offsetY * static_cast<f32>(event.height)),
			.width = static_cast<u32>(mViewport.width * static_cast<f32>(event.width)),
			.height = static_cast<u32>(mViewport.height * static_cast<f32>(event.height))
		};

		mAspectRatio = static_cast<f32>(mWindowExtent.width) / static_cast<f32>(mWindowExtent.height);
	}
}
