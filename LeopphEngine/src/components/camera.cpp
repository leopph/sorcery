#include "camera.h"
#include "../hierarchy/object.h"
#include "../windowing/window.h"
#include "../math/leopphmath.h"
#include <stdexcept>
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



	Camera::Camera() :
		m_AspectRatio{ leopph::impl::Window::Get().AspectRatio() }, m_HorizontalFOVDegrees{ 100.0f }, m_NearClip{ 1.0f }, m_FarClip{ 100.0f }
	{
		if (s_Active == nullptr)
			Activate();
	}

	Camera::~Camera()
	{
		if (s_Active == this)
			s_Active = nullptr;
	}




	// FOV HORIZ-VERT CONVERSION
	float Camera::ConvertFOV(float fov, unsigned char conversion) const
	{
		switch (conversion)
		{
		case VERTICAL_TO_HORIZONTAL:
			return Math::ToDegrees(2.0f * Math::Atan(Math::Tan(Math::ToRadians(fov) / 2.0f) * m_AspectRatio));

		case HORIZONTAL_TO_VERTICAL:
			return Math::ToDegrees(2.0f * Math::Atan(Math::Tan(Math::ToRadians(fov) / 2.0f) / m_AspectRatio));

		default:
			throw std::exception{ "INVALID FOV CONVERSION DIRECTION!" };
		}
	}



	// ASPECT RATIO SETTERS AND GETTERS
	void Camera::AspectRatio(float newRatio)
	{
		m_AspectRatio = newRatio;
	}

	void Camera::AspectRatio(int width, int height)
	{
		m_AspectRatio = static_cast<float>(width) / height;
	}

	float Camera::AspectRatio() const
	{
		return m_AspectRatio;
	}



	// CLIP PLANE SETTERS AND GETTERS
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



	// FOV SETTER AND GETTER
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
			throw std::exception{ "INVALID FOV DIRECTION!" };
		}

	}

	float Camera::FOV(unsigned char direction)
	{
		switch (direction)
		{
		case FOV_HORIZONTAL:
			return m_HorizontalFOVDegrees;

		case FOV_VERTICAL:
			return ConvertFOV(m_HorizontalFOVDegrees, HORIZONTAL_TO_VERTICAL);

		default:
			throw std::exception{ "INVALID FOV DIRECTION!" };
		}
	}



	// CAMERA MATRIX CALCULATIONS
	Matrix4 Camera::ViewMatrix() const
	{
		return Matrix4::LookAt(Object().Transform().Position(), Object().Transform().Position() + Object().Transform().Forward(), Vector3::Up());
	}

	Matrix4 Camera::ProjectionMatrix() const
	{
		float fov{ Math::ToRadians(ConvertFOV(m_HorizontalFOVDegrees, HORIZONTAL_TO_VERTICAL)) };
		return Matrix4::Perspective(fov, m_AspectRatio, m_NearClip, m_FarClip);
	}
}