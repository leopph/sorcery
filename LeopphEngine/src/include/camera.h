#pragma once

#include "leopphapi.h"
#include "vector.h"
#include "matrix.h"
#include "quaternion.h"

namespace leopph
{
#pragma warning(push)
#pragma warning(disable: 4251)

	// CLASS REPRESENTING A CONTROLLABLE CAMERA
	class LEOPPHAPI Camera
	{
	private:
		// COORDINATE-SYSTEM RELATED MEMBERS
		Vector3 m_Position{};
		Quaternion m_Rotation{};
		Vector3 m_Front{ Vector3::Forward() };
		Vector3 m_Upwards{ Vector3::Up() };
		Vector3 m_Right{ Vector3::Right() };

		// PROJECTION RELATED MEMBERS
		float m_AspectRatio{ 1.0f };
		float m_HorizontalFOVDegrees{ 90.0f };
		float m_NearClip{ 0.01f };
		float m_FarClip{ 100.0f };



		// DEFAULT CONSTRUCT ACCORDING TO INITIALIZERS AND NO COPY OR MOVE
		Camera() = default;
		Camera(const Camera&) = delete;

		enum : unsigned char { VERTICAL_TO_HORIZONTAL, HORIZONTAL_TO_VERTICAL };
		float ConvertFOV(float fov, unsigned char conversion) const;


	public:
		enum : unsigned char { FOV_HORIZONTAL, FOV_VERTICAL };
		enum class Movement { FORWARD, BACKWARD, LEFT, RIGHT, DOWN, UP };

		static Camera& Instance();

		void Position(Vector3 newPos);
		const Vector3& Position() const;

		void Rotation(Quaternion newRot);
		const Quaternion& Rotation() const;

		void AspectRatio(float newRatio);
		void AspectRatio(int width, int height);
		float AspectRatio() const;

		void NearClipPlane(float newPlane);
		float NearClipPlane() const;

		void FarClipPlane(float newPlane);
		float FarClipPlane() const;

		void FOV(float fov, unsigned char direction);
		float FOV(unsigned char direction);

		Matrix4 ViewMatrix() const;
		Matrix4 ProjMatrix() const;

		const Vector3& Forward() const;
		const Vector3& Right() const;
		const Vector3& Up() const;
	};

#pragma warning(pop)
}