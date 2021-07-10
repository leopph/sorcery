#pragma once

#include "../math/vector.h"
#include "../api/leopphapi.h"
#include "../math/quaternion.h"
#include <cstddef>

namespace leopph
{
	/*------------------------------------------------------------------
	The Transform class provides Objects with their spatial information.
	They MUST NOT be explicitly instantiated!
	An Object always contains one and only one of these.
	See "object.h" for additional information.
	------------------------------------------------------------------*/

	class Transform
	{
	public:
		/* Internally used constructor */
		LEOPPHAPI Transform();

		/* Spatial position in 3D space */
		LEOPPHAPI const Vector3& Position() const;
		LEOPPHAPI void Position(Vector3 newPos);

		/* Move the Transform relative to its current position */
		LEOPPHAPI void Translate(const Vector3& vector);
		LEOPPHAPI void Translate(float x, float y, float z);

		/* Spatial rotation */
		LEOPPHAPI const Quaternion& Rotation() const;
		LEOPPHAPI void Rotation(Quaternion newRot);

		/* Rotate the Transform around its local axes */
		LEOPPHAPI void RotateLocal(const Quaternion& rotation);
		/* Rotate the transform around the world's axes */
		LEOPPHAPI void RotateGlobal(const Quaternion& rotation);

		/* Spatial scaling */
		LEOPPHAPI const Vector3& Scale() const;
		LEOPPHAPI void Scale(Vector3 newScale);

		/* Scale the Transform relative to its current scaling */
		LEOPPHAPI void Rescale(float x, float y, float z);

		/* Current local axes */
		LEOPPHAPI const Vector3& Forward() const;
		LEOPPHAPI const Vector3& Right() const;
		LEOPPHAPI const Vector3& Up() const;

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