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
		auto normalizedAxis{axis.Normalized()};
		const auto angleHalfRadians = math::ToRadians(angleDegrees) / 2.0f;

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

	auto Quaternion::operator=(const Quaternion& other) -> Quaternion&
	{
		m_W = other.m_W;
		m_X = other.m_X;
		m_Y = other.m_Y;
		m_Z = other.m_Z;
		return *this;
	}

	auto Quaternion::operator=(Quaternion&& other) noexcept -> Quaternion&
	{
		return operator=(other);
	}

	auto Quaternion::operator[](const std::size_t index) const -> const float&
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
				internal::Logger::Instance().Error(errorMsg);
				throw std::out_of_range{errorMsg};
			}
		}
	}

	auto Quaternion::operator[](const std::size_t index) -> float&
	{
		return const_cast<float&>(const_cast<const Quaternion*>(this)->operator[](index));
	}

	auto Quaternion::EulerAngles() const -> Vector3
	{
		// ???

		auto secondComponent{2 * (m_W * m_Y - m_Z * m_X)};

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

	auto Quaternion::Norm() const -> float
	{
		return math::Sqrt(math::Pow(m_W, 2) + math::Pow(m_X, 2) + math::Pow(m_Y, 2) + math::Pow(m_Z, 2));
	}

	auto Quaternion::Normalized() const -> Quaternion
	{
		const auto norm{Norm()};
		return Quaternion{m_W / norm, m_X / norm, m_Y / norm, m_Z / norm};
	}

	auto Quaternion::Normalize() -> Quaternion&
	{
		const auto norm{Norm()};
		m_W /= norm;
		m_X /= norm;
		m_Y /= norm;
		m_Z /= norm;
		return *this;
	}

	auto Quaternion::Conjugate() const -> Quaternion
	{
		return Quaternion{m_W, -m_X, -m_Y, -m_Z};
	}

	auto Quaternion::ConjugateInPlace() -> Quaternion&
	{
		m_X = -m_X;
		m_Y = -m_Y;
		m_Z = -m_Z;
		return *this;
	}

	auto Quaternion::Inverse() const -> Quaternion
	{
		const auto normSquared{math::Pow(m_W, 2) + math::Pow(m_X, 2) + math::Pow(m_Y, 2) + math::Pow(m_Z, 2)};
		return Quaternion{m_W / normSquared, -m_X / normSquared, -m_Y / normSquared, -m_Z / normSquared};
	}

	auto Quaternion::Invert() -> Quaternion&
	{
		const auto normSquared{math::Pow(m_W, 2) + math::Pow(m_X, 2) + math::Pow(m_Y, 2) + math::Pow(m_Z, 2)};
		m_W /= normSquared;
		m_X = -m_X / normSquared;
		m_Y = -m_Y / normSquared;
		m_Z = -m_Z / normSquared;
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

	auto operator*(const Quaternion& left, const Quaternion& right) -> Quaternion
	{
		return Quaternion
		{
			left.W * right.W - left.X * right.X - left.Y * right.Y - left.Z * right.Z,
			left.W * right.X + left.X * right.W + left.Y * right.Z - left.Z * right.Y,
			left.W * right.Y - left.X * right.Z + left.Y * right.W + left.Z * right.X,
			left.W * right.Z + left.X * right.Y - left.Y * right.X + left.Z * right.W
		};
	}

	auto operator*=(Quaternion& left, const Quaternion& right) -> Quaternion&
	{
		return left = left * right;
	}

	auto operator<<(std::ostream& os, const Quaternion& q) -> std::ostream&
	{
		os << "(" << q[0] << ", " << q[1] << ", " << q[2] << ", " << q[3] << ")";
		return os;
	}
}
