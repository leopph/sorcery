#include "Quaternion.hpp"

#include "Math.hpp"

#include <cmath>


namespace leopph
{
	Quaternion::Quaternion(Vector3 const& axis, float const angleDegrees)
	{
		auto normalizedAxis{axis.Normalized()};
		auto const angleHalfRadians = math::ToRadians(angleDegrees) / 2.0f;
		m_W = math::Cos(angleHalfRadians);
		normalizedAxis *= math::Sin(angleHalfRadians);
		m_X = normalizedAxis[0];
		m_Y = normalizedAxis[1];
		m_Z = normalizedAxis[2];
	}


	Vector3 Quaternion::EulerAngles() const
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


	float Quaternion::Norm() const
	{
		return math::Sqrt(math::Pow(m_W, 2) + math::Pow(m_X, 2) + math::Pow(m_Y, 2) + math::Pow(m_Z, 2));
	}


	Quaternion Quaternion::Normalized() const
	{
		auto const norm{Norm()};
		return Quaternion{m_W / norm, m_X / norm, m_Y / norm, m_Z / norm};
	}


	Quaternion& Quaternion::Normalize()
	{
		auto const norm{Norm()};
		m_W /= norm;
		m_X /= norm;
		m_Y /= norm;
		m_Z /= norm;
		return *this;
	}


	Quaternion& Quaternion::ConjugateInPlace()
	{
		m_X = -m_X;
		m_Y = -m_Y;
		m_Z = -m_Z;
		return *this;
	}


	Quaternion Quaternion::Inverse() const
	{
		auto const normSquared{math::Pow(m_W, 2) + math::Pow(m_X, 2) + math::Pow(m_Y, 2) + math::Pow(m_Z, 2)};
		return Quaternion{m_W / normSquared, -m_X / normSquared, -m_Y / normSquared, -m_Z / normSquared};
	}


	Quaternion& Quaternion::Invert()
	{
		auto const normSquared{math::Pow(m_W, 2) + math::Pow(m_X, 2) + math::Pow(m_Y, 2) + math::Pow(m_Z, 2)};
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


	std::ostream& operator<<(std::ostream& os, Quaternion const& q)
	{
		os << "(" << q.W() << ", " << q.X() << ", " << q.Y() << ", " << q.Z() << ")";
		return os;
	}
}
