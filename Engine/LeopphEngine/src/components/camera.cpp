#include "Camera.hpp"

#include "Context.hpp"
#include "Entity.hpp"
#include "Window.hpp"
#include "WindowEvent.hpp"
#include "../rendering/Renderer.hpp"

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



	Matrix4 Camera::build_view_matrix() const
	{
		// inv(T) * inv(R)
		return Matrix4::Translate(-get_owner()->get_position()) * Matrix4{get_owner()->get_rotation()}.Transposed();
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
		mAspectRatio{internal::GetWindowImpl()->get_aspect_ratio()}
	{
		internal::get_renderer()->register_camera(this);
	}



	Camera::~Camera()
	{
		internal::get_renderer()->unregister_camera(this);
	}



	f32 Camera::get_aspect_ratio() const
	{
		return mAspectRatio;
	}



	void Camera::OnEventReceived(EventParamType event)
	{
		mAspectRatio = static_cast<f32>(event.width) / event.height;
	}
}
