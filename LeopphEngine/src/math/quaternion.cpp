#include "Quaternion.hpp"

#include "LeopphMath.hpp"

#include <cmath>


namespace leopph
{
	Quaternion::Quaternion(const Vector3& axis, const float angleDegrees)
	{
		auto normalizedAxis{axis.Normalized()};
		const auto angleHalfRadians = math::ToRadians(angleDegrees) / 2.0f;
		m_W = math::Cos(angleHalfRadians);
		normalizedAxis *= math::Sin(angleHalfRadians);
		m_X = normalizedAxis[0];
		m_Y = normalizedAxis[1];
		m_Z = normalizedAxis[2];
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


	auto operator<<(std::ostream& os, const Quaternion& q) -> std::ostream&
	{
		os << "(" << q.W() << ", " << q.X() << ", " << q.Y() << ", " << q.Z() << ")";
		return os;
	}
}
