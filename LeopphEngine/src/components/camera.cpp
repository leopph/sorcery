#include "camera.h"

#include "../hierarchy/object.h"
#include "../windowing/window.h"
#include "../math/leopphmath.h"

#include "../util/logger.h"

#include <stdexcept>



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



	Camera::Camera() :
		m_AspectRatio{ leopph::impl::Window::Get().AspectRatio() }, m_HorizontalFOVDegrees{ 100.0f }, m_NearClip{ 0.3f }, m_FarClip{ 1000.f }
	{
		if (s_Active == nullptr)
			Activate();
	}

	Camera::~Camera()
	{
		if (s_Active == this)
			s_Active = nullptr;
	}




	float Camera::ConvertFOV(float fov, unsigned char conversion) const
	{
		switch (conversion)
		{
		case VERTICAL_TO_HORIZONTAL:
			return math::ToDegrees(2.0f * math::Atan(math::Tan(math::ToRadians(fov) / 2.0f) * m_AspectRatio));

		case HORIZONTAL_TO_VERTICAL:
			return math::ToDegrees(2.0f * math::Atan(math::Tan(math::ToRadians(fov) / 2.0f) / m_AspectRatio));

		default:
			const auto errorMsg{ "Invalid FOV conversion direction." };
			impl::Logger::Instance().Error(errorMsg);
			throw std::runtime_error{ errorMsg };
		}
	}



	void Camera::AspectRatio(float newRatio)
	{
		m_AspectRatio = newRatio;
	}

	void Camera::AspectRatio(int width, int height)
	{
		m_AspectRatio = static_cast<float>(width) / static_cast<float>(height);
	}

	float Camera::AspectRatio() const
	{
		return m_AspectRatio;
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



	void Camera::FOV(float fov, unsigned char direction)
	{
		switch (direction)
		{
		case FOV_HORIZONTAL:
			m_HorizontalFOVDegrees = fov;
			return;

		case FOV_VERTICAL:
			m_HorizontalFOVDegrees = ConvertFOV(fov, VERTICAL_TO_HORIZONTAL);
			return;

		default:
			auto errorMsg{ "Invalid FOV direction." };
			impl::Logger::Instance().Error(errorMsg);
			throw std::exception{ errorMsg };
		};
	}

	float Camera::FOV(unsigned char direction) const
	{
		switch (direction)
		{
		case FOV_HORIZONTAL:
			return m_HorizontalFOVDegrees;

		case FOV_VERTICAL:
			return ConvertFOV(m_HorizontalFOVDegrees, HORIZONTAL_TO_VERTICAL);

		default:
			const auto errorMsg{ "Invalid FOV direction." };
			impl::Logger::Instance().Error(errorMsg);
			throw std::runtime_error{ errorMsg };
		}
	}



	Matrix4 Camera::ViewMatrix() const
	{
		return Matrix4::LookAt(Object().Transform().Position(), Object().Transform().Position() + Object().Transform().Forward(), Vector3::Up());
	}

	Matrix4 Camera::ProjectionMatrix() const
	{
		float fov{ math::ToRadians(ConvertFOV(m_HorizontalFOVDegrees, HORIZONTAL_TO_VERTICAL)) };
		return Matrix4::Perspective(fov, m_AspectRatio, m_NearClip, m_FarClip);
	}
}