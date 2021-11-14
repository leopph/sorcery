#include "Quaternion.hpp"

#include "LeopphMath.hpp"

#include "../util/logger.h"

#include <cmath>
#include <stdexcept>
#include <string>


namespace leopph
{
	Quaternion::Quaternion(const float w, const float x, const float y, const float z) :
		W{m_W},
		X{m_X},
		Y{m_Y},
		Z{m_Z},
		m_W{w},
		m_X{x},
		m_Y{y},
		m_Z{z}
	{}

	Quaternion::Quaternion(const Vector3& axis, const float angleDegrees) :
		W{m_W},
		X{m_X},
		Y{m_Y},
		Z{m_Z}
	{
		Vector3 normalizedAxis{axis.Normalized()};
		const float angleHalfRadians = math::ToRadians(angleDegrees) / 2.0f;

		m_W = math::Cos(angleHalfRadians);
		normalizedAxis *= math::Sin(angleHalfRadians);
		m_X = normalizedAxis[0];
		m_Y = normalizedAxis[1];
		m_Z = normalizedAxis[2];
	}

	Quaternion::Quaternion(const Quaternion& other) :
		W{m_W},
		X{m_X},
		Y{m_Y},
		Z{m_Z},
		m_W{other.m_W},
		m_X{other.m_X},
		m_Y{other.m_Y},
		m_Z{other.m_Z}
	{}

	Quaternion::Quaternion(Quaternion&& other) noexcept :
		Quaternion{other}
	{}

	Quaternion& Quaternion::operator=(const Quaternion& other)
	{
		m_W = other.m_W;
		m_X = other.m_X;
		m_Y = other.m_Y;
		m_Z = other.m_Z;
		return *this;
	}

	Quaternion& Quaternion::operator=(Quaternion&& other) noexcept
	{
		return operator=(other);
	}

	const float& Quaternion::operator[](const std::size_t index) const
	{
		switch (index)
		{
			case 0:
			{
				return m_W;
			}
			case 1:
			{
				return m_X;
			}
			case 2:
			{
				return m_Y;
			}
			case 3:
			{
				return m_Z;
			}
			default:
			{
				const auto errorMsg{"Invalid quaternion index '" + std::to_string(index) + "'."};
				impl::Logger::Instance().Error(errorMsg);
				throw std::out_of_range{errorMsg};
			}
		}
	}

	float& Quaternion::operator[](const std::size_t index)
	{
		return const_cast<float&>(const_cast<const Quaternion*>(this)->operator[](index));
	}

	Vector3 Quaternion::EulerAngles() const
	{
		// ???

		float secondComponent{2 * (m_W * m_Y - m_Z * m_X)};

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
			math::ToDegrees(math::Atan2(2 * (m_W * m_X + m_Y * m_Z), 1 - 2 * (math::Pow(m_X, 2) + math::Pow(m_Y, 2)))),
			math::ToDegrees(secondComponent),
			math::ToDegrees(math::Atan2(2 * (m_W * m_Z + m_X * m_Y), 1 - 2 * (math::Pow(m_Y, 2) + math::Pow(m_Z, 2))))
		};
	}

	float Quaternion::Norm() const
	{
		return math::Sqrt(math::Pow(m_W, 2) + math::Pow(m_X, 2) + math::Pow(m_Y, 2) + math::Pow(m_Z, 2));
	}

	Quaternion Quaternion::Conjugate() const
	{
		return Quaternion{m_W, -m_X, -m_Y, -m_Z};
	}

	Quaternion& Quaternion::ConjugateInPlace()
	{
		m_X = -m_X;
		m_Y = -m_Y;
		m_Z = -m_Z;
		return *this;
	}

	Quaternion::operator Matrix4() const
	{
		return Matrix4
		{
			1 - 2 * (math::Pow(m_Y, 2) + math::Pow(m_Z, 2)), 2 * (m_X * m_Y + m_Z * m_W), 2 * (m_X * m_Z - m_Y * m_W), 0,
			2 * (m_X * m_Y - m_Z * m_W), 1 - 2 * (math::Pow(m_X, 2) + math::Pow(m_Z, 2)), 2 * (m_Y * m_Z + m_X * m_W), 0,
			2 * (m_X * m_Z + m_Y * m_W), 2 * (m_Y * m_Z - m_X * m_W), 1 - 2 * (math::Pow(m_X, 2) + math::Pow(m_Y, 2)), 0,
			0, 0, 0, 1
		};
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