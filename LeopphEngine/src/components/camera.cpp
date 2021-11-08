#include "Camera.hpp"

#include "../entity/Entity.hpp"
#include "../events/ScreenResolutionEvent.hpp"
#include "../math/LeopphMath.hpp"
#include "../util/logger.h"
#include "../windowing/WindowBase.hpp"

#include <stdexcept>
#include <utility>


namespace leopph
{
	Camera* Camera::s_Active{nullptr};
	Camera* const& Camera::Active{s_Active};


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
		impl::WindowBase::Get().Background(m_Background.color);
	}


	Camera::Camera(leopph::Entity& owner) :
		Component{owner},
		m_AspectRatio{leopph::impl::WindowBase::Get().AspectRatio()},
		m_HorizontalFovDegrees{100.0f},
		m_NearClip{0.1f},
		m_FarClip{100.f},
		m_Background{.color{0, 0, 0}, .skybox{}}
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


	float Camera::ConvertFov(const float fov, const FovConversionDirection conversion) const
	{
		if (conversion == FovConversionDirection::VerticalToHorizontal)
		{
			return math::ToDegrees(2.0f * math::Atan(math::Tan(math::ToRadians(fov) / 2.0f) * m_AspectRatio));
		}

		if (conversion == FovConversionDirection::HorizontalToVertical)
		{
			return math::ToDegrees(2.0f * math::Atan(math::Tan(math::ToRadians(fov) / 2.0f) / m_AspectRatio));
		}

		const auto errMsg{"Invalid FOV conversion direction."};
		impl::Logger::Instance().Critical(errMsg);
		throw std::invalid_argument{errMsg};
	}


	void Camera::NearClipPlane(const float newPlane)
	{
		m_NearClip = newPlane;
	}


	float Camera::NearClipPlane() const
	{
		return m_NearClip;
	}


	void Camera::FarClipPlane(const float newPlane)
	{
		m_FarClip = newPlane;
	}


	float Camera::FarClipPlane() const
	{
		return m_FarClip;
	}


	void Camera::Fov(const float degrees, const FovDirection direction)
	{
		if (direction == FovDirection::Horizontal)
		{
			m_HorizontalFovDegrees = degrees;
		}
		else if (direction == FovDirection::Vertical)
		{
			m_HorizontalFovDegrees = ConvertFov(degrees, FovConversionDirection::VerticalToHorizontal);
		}
	}


	float Camera::Fov(const FovDirection direction) const
	{
		if (direction == FovDirection::Horizontal)
		{
			return m_HorizontalFovDegrees;
		}
		if (direction == FovDirection::Vertical)
		{
			return ConvertFov(m_HorizontalFovDegrees, FovConversionDirection::HorizontalToVertical);
		}

		const auto errMsg{"Invalid FOV direction."};
		impl::Logger::Instance().Critical(errMsg);
		throw std::invalid_argument{errMsg};
	}


	Matrix4 Camera::ViewMatrix() const
	{
		return (static_cast<Matrix4>(Entity.Transform->Rotation()) * Matrix4::Translate(Entity.Transform->Position())).Inverse();
	}


	Matrix4 Camera::ProjectionMatrix() const
	{
		const auto fov{math::ToRadians(ConvertFov(m_HorizontalFovDegrees, FovConversionDirection::HorizontalToVertical))};
		return Matrix4::Perspective(fov, m_AspectRatio, m_NearClip, m_FarClip);
	}


	void Camera::OnEventReceived(EventParamType event)
	{
		m_AspectRatio = event.NewResolution[0] / event.NewResolution[1];
	}
}
