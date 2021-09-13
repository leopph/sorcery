#include "Camera.hpp"

#include "../entity/Entity.hpp"
#include "../events/DisplayResolutionChangedEvent.hpp"
#include "../math/LeopphMath.hpp"
#include "../windowing/window.h"

#include <utility>



namespace leopph
{
	Camera* Camera::s_Active = nullptr;


	Camera* Camera::Active()
	{
		return s_Active;
	}


	void Camera::Activate()
	{
		s_Active = this;
	}


	const leopph::CameraBackground& Camera::Background() const
	{
		return m_Background;
	}


	void Camera::Background(CameraBackground&& background)
	{
		m_Background.color = background.color;
		m_Background.skybox = std::move(background.skybox);
		impl::Window::Get().Background(m_Background.color);
	}


	Camera::Camera(Entity& owner) :
		Component{owner},
		m_AspectRatio{leopph::impl::Window::Get().AspectRatio()},
		m_HorizontalFOVDegrees{100.0f},
		m_NearClip{0.1f},
		m_FarClip{100.f},
		m_Background{.color{0, 0, 0}, .skybox{nullptr}}
	{
		if (s_Active == nullptr)
		{
			Activate();
		}
	}


	Camera::~Camera()
	{
		if (s_Active == this)
		{
			s_Active = nullptr;
		}
	}


	float Camera::ConvertFOV(const float fov, FovConversionDirection conversion) const
	{
		if (conversion == FovConversionDirection::VerticalToHorizontal)
		{
			return math::ToDegrees(2.0f * math::Atan(math::Tan(math::ToRadians(fov) / 2.0f) * m_AspectRatio));
		}

		if (conversion == FovConversionDirection::HorizontalToVertical)
		{
			return math::ToDegrees(2.0f * math::Atan(math::Tan(math::ToRadians(fov) / 2.0f) / m_AspectRatio));
		}
	}


	void Camera::NearClipPlane(float newPlane)
	{
		m_NearClip = newPlane;
	}


	float Camera::NearClipPlane() const
	{
		return m_NearClip;
	}


	void Camera::FarClipPlane(float newPlane)
	{
		m_FarClip = newPlane;
	}


	float Camera::FarClipPlane() const
	{
		return m_FarClip;
	}


	void Camera::Fov(const float fov, const FovDirection direction)
	{
		if (direction == FovDirection::Horizontal)
		{
			m_HorizontalFOVDegrees = fov;
		}
		else if (direction == FovDirection::Vertical)
		{
			m_HorizontalFOVDegrees = ConvertFOV(fov, FovConversionDirection::VerticalToHorizontal);
		}
	}


	float Camera::Fov(const FovDirection direction) const
	{
		if (direction == FovDirection::Horizontal)
		{
			return m_HorizontalFOVDegrees;
		}
		if (direction == FovDirection::Vertical)
		{
			return ConvertFOV(m_HorizontalFOVDegrees, FovConversionDirection::HorizontalToVertical);
		}
	}


	Matrix4 Camera::ViewMatrix() const
	{
		return (static_cast<Matrix4>(entity.Transform->Rotation()) * Matrix4::Translate(entity.Transform->Position())).Inverse();
		//return Matrix4::LookAt(object.Transform->Position(), object.Transform->Position() + object.Transform->Forward(), Vector3::Up());
	}


	Matrix4 Camera::ProjectionMatrix() const
	{
		const auto fov{math::ToRadians(ConvertFOV(m_HorizontalFOVDegrees, FovConversionDirection::HorizontalToVertical))};
		return Matrix4::Perspective(fov, m_AspectRatio, m_NearClip, m_FarClip);
	}


	void Camera::OnEventReceived(EventParamType event)
	{
		m_AspectRatio = event.newResolution[0] / event.newResolution[1];
	}
}
