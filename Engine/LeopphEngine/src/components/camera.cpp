#include "Camera.hpp"

#include "Entity.hpp"
#include "WindowEvent.hpp"
#include "../InternalContext.hpp"
#include "../windowing/WindowImpl.hpp"

#include <utility>


namespace leopph
{
	Camera* Camera::Current()
	{
		return s_Current;
	}



	void Camera::MakeCurrent()
	{
		if (Active())
		{
			s_Current = this;
		}
	}



	void Camera::NearClipPlane(float const newPlane)
	{
		m_NearClip = newPlane;
	}



	float Camera::NearClipPlane() const
	{
		return m_NearClip;
	}



	void Camera::FarClipPlane(float const newPlane)
	{
		m_FarClip = newPlane;
	}



	float Camera::FarClipPlane() const
	{
		return m_FarClip;
	}



	std::variant<Color, Skybox> const& Camera::Background() const
	{
		return m_Background;
	}



	void Camera::Background(std::variant<Color, Skybox> background)
	{
		m_Background = std::move(background);

		if (std::holds_alternative<Color>(m_Background))
		{
			internal::GetWindowImpl()->ClearColor(static_cast<Vector4>(std::get<Color>(m_Background)));
		}
	}



	Matrix4 Camera::ViewMatrix() const
	{
		// inv(T) * inv(R)
		return Matrix4::Translate(-Owner()->get_transform().get_position()) * Matrix4{Owner()->get_transform().get_rotation()}.Transposed();
	}



	Vector2 Camera::TransformToViewport(Vector3 const& vector) const noexcept
	{
		Vector4 homoPos{vector, 1};
		homoPos *= ViewMatrix() * ProjectionMatrix();
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



	void Camera::Owner(Entity* entity)
	{
		Component::Owner(entity);

		if (!InUse() && s_Current == this)
		{
			s_Current = nullptr;
		}
	}



	void Camera::Active(bool const active)
	{
		Component::Active(active);

		if (!InUse() && s_Current == this)
		{
			s_Current = nullptr;
		}
	}



	Camera::~Camera()
	{
		if (s_Current == this)
		{
			s_Current = nullptr;
		}
	}



	Camera::Camera() :
		m_AspectRatio{internal::GetWindowImpl()->AspectRatio()},
		m_Background{Color{internal::GetWindowImpl()->ClearColor()}}
	{
		if (s_Current == nullptr)
		{
			MakeCurrent();
		}
	}



	float Camera::AspectRatio() const
	{
		return m_AspectRatio;
	}



	void Camera::OnEventReceived(EventParamType event)
	{
		m_AspectRatio = static_cast<float>(event.Width) / event.Height;
	}



	Camera* Camera::s_Current;
}
