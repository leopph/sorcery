#include "quaternion.h"
#include "leopphmath.h"

namespace leopph
{
	Quaternion::Quaternion(float w, float x, float y, float z) :
		w{ w }, x{ x }, y{ y }, z{ z }
	{
		float length = Magnitude();
		if (length != 1)
		{
			w /= length;
			x /= length;
			y /= length;
			z /= length;
		}
	}


	Quaternion::Quaternion(const Vector3& axis, float angleDegrees)
	{
		Vector3 normalizedAxis = axis.Normalized();
		float angleHalfRadians = Math::ToRadians(angleDegrees) / 2.0f;

		w = Math::Cos(angleHalfRadians);
		x = normalizedAxis[0] * Math::Sin(angleHalfRadians);
		y = normalizedAxis[1] * Math::Sin(angleHalfRadians);
		z = normalizedAxis[2] * Math::Sin(angleHalfRadians);
	}





	const float& Quaternion::operator[](std::size_t index) const
	{
		switch (index)
		{
		case 0:
			return w;
		case 1:
			return x;
		case 2:
			return y;
		case 3:
			return z;
		default:
			throw std::out_of_range{ "Invalid quaternion index '" + std::to_string(index) + "'" };
		}
	}

	float& Quaternion::operator[](std::size_t index)
	{
		return const_cast<float&>(const_cast<const Quaternion*>(this)->operator[](index));
	}




	Quaternion::operator Matrix4() const
	{
		return Matrix4
		{
			1 - 2 * (Math::Pow(y, 2) + Math::Pow(z, 2)),		2 * (x * y + z * w),		2 * (x * z - y * w),		0,
			2 * (x * y - z * w),		1 - 2 * (Math::Pow(x, 2) + Math::Pow(z, 2)),		2 * (y * z + x * w),		0,
			2 * (x * z + y * w),		2 * (y * z - x * w),		1 - 2 * (Math::Pow(x, 2) + Math::Pow(y, 2)),		0,
			0,		0,		0,		1
		};
	}





	float Quaternion::Magnitude() const
	{
		return Math::Sqrt(Math::Pow(w, 2) + Math::Pow(x, 2) + Math::Pow(y, 2) + Math::Pow(z, 2));
	}





	Quaternion operator*(const Quaternion& left, const Quaternion& right)
	{
		return Quaternion
		{
			left[0] * right[0] - left[1] * right[1] - left[2] * right[2] - left[3] * right[3],
			left[0] * right[1] + left[1] * right[0] + left[2] * right[3] - left[3] * right[2],
			left[0] * right[2] - left[1] * right[3] + left[2] * right[0] + left[3] * right[1],
			left[0] * right[3] + left[1] * right[2] - left[2] * right[1] + left[3] * right[0]
		};
	}




	std::ostream& operator<<(std::ostream& os, const Quaternion& q)
	{
		os << "(" << q[0] << ", " << q[1] << ", " << q[2] << ", " << q[3] << ")";
		return os;
	}
}