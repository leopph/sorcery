#pragma once


#include "leopphapi.h"
#include "vector.h"
#include "matrix.h"
#include <cstddef>
#include <stdexcept>
#include <string>
#include <ostream>


namespace leopph
{
	class LEOPPHAPI Quaternion
	{
	public:
		// constructors
		Quaternion(float w = 1.0f, float x = 0.0f, float y = 0.0f, float z = 0.0f);
		Quaternion(const Vector3& axis, float angle);


		// order of elements is w x y z
		const float& operator[](std::size_t index) const;
		float& operator[](std::size_t index);

		float Magnitude() const;


		// rotation matrix
		operator Matrix4() const;


	private:
		float w{ 1.0f };
		float x{ 0.0f };
		float y{ 0.0f };
		float z{ 0.0f };
	};


	LEOPPHAPI Quaternion operator*(const Quaternion& left, const Quaternion& right);
	LEOPPHAPI std::ostream& operator<<(std::ostream& os, const Quaternion& q);
}