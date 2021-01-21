#include "camera.h"

#include <stdexcept>
#include <glm/gtc/matrix_transform.hpp>

namespace leopph
{
	// POS GETTER
	const glm::vec3& Camera::Position() const
	{
		return m_Position;
	}



	// FOV HORIZ-VERT CONVERSION
	float Camera::ConvertFOV(float fov, unsigned char conversion) const
	{
		switch (conversion)
		{
		case VERTICAL_TO_HORIZONTAL:
			return glm::degrees(2.0f * std::atan(std::tan(glm::radians(fov) / 2.0f) * m_AspectRatio));

		case HORIZONTAL_TO_VERTICAL:
			return glm::degrees(2.0f * std::atan(std::tan(glm::radians(fov) / 2.0f) / m_AspectRatio));

		default:
			throw std::exception{ "INVALID FOV CONVERSION DIRECTION!" };
		}
	}



	// CALCULATING NEW COORDINATE SYSTEM AXES
	void Camera::UpdateVectors()
	{
		glm::vec3 newFront;
		newFront.x = static_cast<float>(std::cos(glm::radians(m_Yaw)) * std::cos(glm::radians(m_Pitch)));
		newFront.y = static_cast<float>(std::sin(glm::radians(m_Pitch)));
		newFront.z = static_cast<float>(std::sin(glm::radians(m_Yaw)) * std::cos(glm::radians(m_Pitch)));

		m_Front = glm::normalize(newFront);
		m_Right = glm::normalize(glm::cross(m_Front, m_WorldUpwards));
		m_Upwards = glm::normalize(glm::cross(m_Right, m_Front));
	}



	// YAW AND PITCH SETTERS TO HANDLE CONSTRAINTS
	void Camera::SetYaw(double newYaw)
	{
		m_Yaw = newYaw;
	}

	void Camera::SetPitch(double newPitch)
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
	glm::mat4 Camera::ViewMatrix() const
	{
		return glm::lookAt(m_Position, m_Position + m_Front, m_Upwards);
	}

	glm::mat4 Camera::ProjMatrix() const
	{
		float fov{ glm::radians(ConvertFOV(m_HorizontalFOVDegrees, HORIZONTAL_TO_VERTICAL)) };
		return glm::perspective(fov, m_AspectRatio, m_NearClip, m_FarClip);
	}



	// INPUT HANDLING
	void Camera::ProcessKeyboardInput(Movement direction, float deltaTime)
	{
		switch (direction)
		{
		case Movement::FORWARD:
			m_Position += (m_Front - glm::dot(m_Front, m_WorldUpwards) * m_WorldUpwards) * m_Speed * deltaTime;
			return;
		case Movement::BACKWARD:
			m_Position -= (m_Front - glm::dot(m_Front, m_WorldUpwards) * m_WorldUpwards) * m_Speed * deltaTime;
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

	void Camera::ProcessMouseInput(double offsetX, double offsetY)
	{
		offsetX *= m_MouseSens;
		offsetY *= m_MouseSens;

		SetYaw(m_Yaw + offsetX);
		SetPitch(m_Pitch + offsetY);

		UpdateVectors();
	}
}