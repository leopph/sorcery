#include "camera.h"
#include "leopphmath.h"
#include <stdexcept>
#include <utility>



namespace leopph
{
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



	// CALCULATING NEW COORDINATE SYSTEM AXES
	void Camera::UpdateVectors()
	{
		Vector3 newFront;
		newFront[0] = static_cast<float>(Math::Cos(Math::ToRadians(m_Yaw)) * Math::Cos(Math::ToRadians(m_Pitch)));
		newFront[1] = static_cast<float>(Math::Sin(Math::ToRadians(m_Pitch)));
		newFront[2] = static_cast<float>(Math::Sin(Math::ToRadians(m_Yaw)) * Math::Cos(Math::ToRadians(m_Pitch)));

		m_Front = newFront.Normalized();
		m_Right = Vector3::Cross(m_Front, Vector3::Up()).Normalized();
		m_Upwards = Vector3::Cross(m_Right, m_Front).Normalized();
	}



	// SINGLETON INSTANCE
	Camera& Camera::Instance()
	{
		static Camera instance;
		return instance;
	}



	// SETTERS AND GETTERS
	void Camera::Position(Vector3 newPos)
	{
		m_Position = std::move(newPos);
	}

	const Vector3& Camera::Position() const
	{
		return m_Position;
	}

	void Camera::Rotation(Quaternion newRot)
	{
		m_Rotation = std::move(newRot);

		Matrix3 rotationMatrix{ static_cast<Matrix3>(static_cast<Matrix4>(newRot)) };
		m_Front = Vector3::Forward() * rotationMatrix;
		m_Upwards = Vector3::Up() * rotationMatrix;
		m_Right = Vector3::Right() * rotationMatrix;
	}

	const Quaternion& Camera::Rotation() const
	{
		return m_Rotation;
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
		return Matrix4::LookAt(m_Position, m_Position + m_Front, m_Upwards);
	}

	Matrix4 Camera::ProjMatrix() const
	{
		float fov{ Math::ToRadians(ConvertFOV(m_HorizontalFOVDegrees, HORIZONTAL_TO_VERTICAL)) };
		return Matrix4::Perspective(fov, m_AspectRatio, m_NearClip, m_FarClip);
	}
}