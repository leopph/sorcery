#include "camera.h"
#include "leopphmath.h"

#include <stdexcept>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace leopph
{
	// POS GETTER
	const Vector3& Camera::Position() const
	{
		return m_Position;
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



	// CALCULATING NEW COORDINATE SYSTEM AXES
	void Camera::UpdateVectors()
	{
		Vector3 newFront;
		newFront[0] = static_cast<float>(Math::Cos(Math::ToRadians(m_Yaw)) * Math::Cos(Math::ToRadians(m_Pitch)));
		newFront[1] = static_cast<float>(Math::Sin(Math::ToRadians(m_Pitch)));
		newFront[2] = static_cast<float>(Math::Sin(Math::ToRadians(m_Yaw)) * Math::Cos(Math::ToRadians(m_Pitch)));

		m_Front = newFront.Normalized();
		m_Right = Vector3::Cross(m_Front, m_WorldUpwards).Normalized();
		m_Upwards = Vector3::Cross(m_Right, m_Front).Normalized();
	}



	// YAW AND PITCH SETTERS TO HANDLE CONSTRAINTS
	void Camera::SetYaw(float newYaw)
	{
		m_Yaw = newYaw;
	}

	void Camera::SetPitch(float newPitch)
	{
		if (newPitch > PITCH_CONSTRAINT)
			m_Pitch = PITCH_CONSTRAINT;
		else if (newPitch < -PITCH_CONSTRAINT)
			m_Pitch = -PITCH_CONSTRAINT;
		else
			m_Pitch = newPitch;
	}



	// SINGLETON INSTANCE
	Camera& Camera::Instance()
	{
		static Camera instance;
		return instance;
	}



	// CAMERA SPEED SETTERS AND GETTERS
	void Camera::Speed(float newSpeed)
	{
		m_Speed = newSpeed;
	}

	float Camera::Speed() const
	{
		return m_Speed;
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
		glm::vec3 pos{ m_Position[0], m_Position[1], m_Position[2] };
		glm::vec3 front{ m_Front[0], m_Front[1], m_Front[2] };
		glm::vec3 up{ m_Upwards[0], m_Upwards[1], m_Upwards[2] };

		auto lookat = glm::lookAt(pos, pos + front, up);

		Matrix4 ret;

		for (size_t i = 0; i < 4; i++)
			for (size_t j = 0; j < 4; j++)
				ret[i][j] = lookat[j][i];

		return ret;

		//return Matrix4::LookAt(m_Position, m_Front, m_Upwards);
	}

	Matrix4 Camera::ProjMatrix() const
	{
		float fov{ Math::ToRadians(ConvertFOV(m_HorizontalFOVDegrees, HORIZONTAL_TO_VERTICAL)) };
		//return Matrix4::Perspective(fov, m_AspectRatio, m_NearClip, m_FarClip);

		auto pers = glm::perspective(fov, m_AspectRatio, m_NearClip, m_FarClip);

		Matrix4 ret;

		for (size_t i = 0; i < 4; i++)
			for (size_t j = 0; j < 4; j++)
				ret[i][j] = pers[i][j];

		return ret;
	}



	// INPUT HANDLING
	void Camera::ProcessKeyboardInput(Movement direction, float deltaTime)
	{
		switch (direction)
		{
		case Movement::FORWARD:
			m_Position += (m_Front - Vector3::Dot(m_Front, m_WorldUpwards) * m_WorldUpwards) * m_Speed * deltaTime;
			return;
		case Movement::BACKWARD:
			m_Position -= (m_Front - Vector3::Dot(m_Front, m_WorldUpwards) * m_WorldUpwards) * m_Speed * deltaTime;
			return;
		case Movement::RIGHT:
			m_Position += m_Right * m_Speed * deltaTime;
			return;
		case Movement::LEFT:
			m_Position -= m_Right * m_Speed * deltaTime;
			return;
		case Movement::UP:
			m_Position += m_WorldUpwards * m_Speed * deltaTime;
			return;
		case Movement::DOWN:
			m_Position -= m_WorldUpwards * m_Speed * deltaTime;
		}
	}

	void Camera::ProcessMouseInput(float offsetX, float offsetY)
	{
		offsetX *= m_MouseSens;
		offsetY *= m_MouseSens;

		SetYaw(m_Yaw + offsetX);
		SetPitch(m_Pitch + offsetY);

		UpdateVectors();
	}
}