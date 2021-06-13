#pragma once

#include "../math/vector.h"
#include "../api/leopphapi.h"
#include "../math/quaternion.h"
#include <cstddef>



namespace leopph
{
	class Transform
	{
	public:
		LEOPPHAPI Transform();

		LEOPPHAPI const Vector3& Position() const;
		LEOPPHAPI void Position(Vector3 newPos);

		LEOPPHAPI void Translate(const Vector3& vector);
		LEOPPHAPI void Translate(float x, float y, float z);

		LEOPPHAPI const Quaternion& Rotation() const;
		LEOPPHAPI void Rotation(Quaternion newRot);

		LEOPPHAPI void RotateLocal(const Quaternion& rotation);
		LEOPPHAPI void RotateGlobal(const Quaternion& rotation);

		LEOPPHAPI const Vector3& Scale() const;
		LEOPPHAPI void Scale(Vector3 newScale);

		LEOPPHAPI void Rescale(float x, float y, float z);

		LEOPPHAPI const Vector3& Forward() const;
		LEOPPHAPI const Vector3& Right() const;
		LEOPPHAPI const Vector3& Up() const;


		// prevent dynamic allocation by clients
		void* operator new(std::size_t) = delete;
		void* operator new[](std::size_t) = delete;


	private:
		Vector3 m_Position;
		Quaternion m_Rotation;
		Vector3 m_Scale;

		Vector3 m_Forward;
		Vector3 m_Right;
		Vector3 m_Up;
	};
}