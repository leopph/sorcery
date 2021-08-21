#include "Quaternion.hpp"

#include "LeopphMath.hpp"

#include "../util/logger.h"

#include <cmath>
#include <stdexcept>
#include <string>


namespace leopph
{
	/*---------------
	 MEMBER FUNCTIONS
	 --------------*/

	Quaternion::Quaternion(const float w, const float x, const float y, const float z) :
		w{ w }, x{ x }, y{ y }, z{ z }
	{
		if (const float length = Magnitude();
			length != 1)
		{
			this->w /= length;
			this->x /= length;
			this->y /= length;
			this->z /= length;
		}
	}


	Quaternion::Quaternion(const Vector3& axis, const float angleDegrees)
	{
		Vector3 normalizedAxis = axis.Normalized();
		const float angleHalfRadians = math::ToRadians(angleDegrees) / 2.0f;

		w = math::Cos(angleHalfRadians);
		x = normalizedAxis[0] * math::Sin(angleHalfRadians);
		y = normalizedAxis[1] * math::Sin(angleHalfRadians);
		z = normalizedAxis[2] * math::Sin(angleHalfRadians);
	}


	Vector3 Quaternion::EulerAngles() const
	{
		float secondComponent{ 2 * (w * y - z * x) };

		if (math::Abs(secondComponent) > 1)
		{
			secondComponent = std::copysign(math::Pi() / 2, secondComponent);
		}
		else
		{
			secondComponent = math::Asin(secondComponent);
		}

		return Vector3
		{
			math::ToDegrees(math::Atan2(2 * (w * x + y * z), 1 - 2 * (math::Pow(x, 2) + math::Pow(y, 2)))),
			math::ToDegrees(secondComponent),
			math::ToDegrees(math::Atan2(2 * (w * z + x * y), 1 - 2 * (math::Pow(y, 2) + math::Pow(z, 2))))
		};
	}


	const float& Quaternion::operator[](const std::size_t index) const
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
			const auto errorMsg{ "Invalid quaternion index '" + std::to_string(index) + "'." };
			impl::Logger::Instance().Error(errorMsg);
			throw std::out_of_range{ errorMsg };
		}
	}


	float& Quaternion::operator[](const std::size_t index)
	{
		return const_cast<float&>(const_cast<const Quaternion*>(this)->operator[](index));
	}


	Quaternion::operator Matrix4() const
	{
		return Matrix4
		{
			1 - 2 * (math::Pow(y, 2) + math::Pow(z, 2)),		2 * (x * y + z * w),		2 * (x * z - y * w),		0,
			2 * (x * y - z * w),		1 - 2 * (math::Pow(x, 2) + math::Pow(z, 2)),		2 * (y * z + x * w),		0,
			2 * (x * z + y * w),		2 * (y * z - x * w),		1 - 2 * (math::Pow(x, 2) + math::Pow(y, 2)),		0,
			0,		0,		0,		1
		};
	}


	float Quaternion::Magnitude() const
	{
		return math::Sqrt(math::Pow(w, 2) + math::Pow(x, 2) + math::Pow(y, 2) + math::Pow(z, 2));
	}


	/*-------------------
	 NON-MEMBER FUNCTIONS
	 ------------------*/

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


	Quaternion& operator*=(Quaternion& left, const Quaternion& right)
	{
		return left = left * right;
	}


	std::ostream& operator<<(std::ostream& os, const Quaternion& q)
	{
		os << "(" << q[0] << ", " << q[1] << ", " << q[2] << ", " << q[3] << ")";
		return os;
	}
}