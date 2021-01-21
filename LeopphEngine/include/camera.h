#pragma once

#include "leopph.h"

#include <glm/glm.hpp>

namespace leopph
{
#pragma warning(push)
#pragma warning(disable: 4251)

	// CLASS REPRESENTING A CONTROLLABLE CAMERA
	class LEOPPHAPI Camera
	{
	private:
		// COORDINATE-SYSTEM RELATED MEMBERS
		glm::vec3 m_Position{};
		glm::vec3 m_Front{ 0.0f, 0.0f, -1.0f };
		glm::vec3 m_Upwards{ 0.0f, 1.0f, 0.0f };
		glm::vec3 m_Right{ 1.0f, 0.0f, 0.0f };
		const glm::vec3 m_WorldUpwards{ 0.0f, 1.0f, 0.0f };

		// PROJECTION RELATED MEMBERS
		float m_AspectRatio{ 1.0f };
		float m_HorizontalFOVDegrees{ 90.0f };
		float m_NearClip{ 0.01f };
		float m_FarClip{ 100.0f };
		double m_Yaw{ -90.0f };
		double m_Pitch{};
		const float PITCH_CONSTRAINT{ 89.0f };

		// CONTROL RELATED MEMBERS
		float m_Speed{ 1.0f };
		double m_MouseSens{ 0.1f };



		// DEFAULT CONSTRUCT ACCORDING TO INITIALIZERS AND NO COPY OR MOVE
		Camera() = default;
		Camera(const Camera&) = delete;

		enum : unsigned char { VERTICAL_TO_HORIZONTAL, HORIZONTAL_TO_VERTICAL };
		float ConvertFOV(float fov, unsigned char conversion) const;

		void UpdateVectors();

		void SetPitch(double newPitch);
		void SetYaw(double newYaw);



	public:
		enum : unsigned char { FOV_HORIZONTAL, FOV_VERTICAL };
		enum class Movement { FORWARD, BACKWARD, LEFT, RIGHT, DOWN, UP };

		static Camera& Instance();

		const glm::vec3& Position() const;

		void Speed(float newSpeed);
		float Speed() const;

		void AspectRatio(float newRatio);
		void AspectRatio(int width, int height);
		float AspectRatio() const;

		void NearClipPlane(float newPlane);
		float NearClipPlane() const;

		void FarClipPlane(float newPlane);
		float FarClipPlane() const;

		void FOV(float fov, unsigned char direction);
		float FOV(unsigned char direction);

		glm::mat4 ViewMatrix() const;
		glm::mat4 ProjMatrix() const;

		void ProcessKeyboardInput(Movement direction, float deltaTime);
		void ProcessMouseInput(double offsetX, double offsetY);
	};

#pragma warning(pop)
}