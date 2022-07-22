#pragma once

#include "LeopphApi.hpp"
#include "Matrix.hpp"
#include "Vector.hpp"

#include <ostream>


namespace leopph
{
	// Quaternions are a 4 dimensional extension to complex numbers. In LeopphEngine, they are used to calculate rotations.
	class Quaternion
	{
		public:
			// Create a Quaternion by its real and imaginary components.
			constexpr explicit Quaternion(float w = 1.0f, float x = 0.0f, float y = 0.0f, float z = 0.0f);

			// Create a Quaternion that rotates 'angleDegrees' amount around the 'axis' axis.
			// 'Axis' does not need to be a unit vector.
			// The Quaternion will have a norm of 1.
			LEOPPHAPI explicit Quaternion(Vector3 const& axis, float angleDegrees);

			constexpr Quaternion(Quaternion const& other) = default;
			constexpr Quaternion& operator=(Quaternion const& other) = default;

			constexpr Quaternion(Quaternion&& other) = default;
			constexpr Quaternion& operator=(Quaternion&& other) = default;

			constexpr ~Quaternion() = default;

			// Get the Quaternion's real part.
			[[nodiscard]] constexpr auto W() const noexcept;
			// Get the Quaternion's coefficient of i.
			[[nodiscard]] constexpr auto X() const noexcept;
			// Get the Quaternion's coefficient of j.
			[[nodiscard]] constexpr auto Y() const noexcept;
			// Get the Quaternion's coefficient of k.
			[[nodiscard]] constexpr auto Z() const noexcept;

			// Set the Quaternion's real part.
			constexpr auto W(float w) noexcept;
			// Set the Quaternion's coefficient of i.
			constexpr auto X(float x) noexcept;
			// Set the Quaternion's coefficient of j.
			constexpr auto Y(float y) noexcept;
			// Set the Quaternion's coefficient of k.
			constexpr auto Z(float z) noexcept;

			[[nodiscard]] LEOPPHAPI Vector3 EulerAngles() const;

			// Get the norm of the Quaternion.
			[[nodiscard]] LEOPPHAPI float Norm() const;

			// Return a Quaternion that represents the same orientation, but has a norm of 1.
			[[nodiscard]] LEOPPHAPI Quaternion Normalized() const;

			// Modifies this Quaternion so that it represents the same orientation, but has a norm of 1.
			// Returns a reference to this Quaternion.
			LEOPPHAPI Quaternion& Normalize();

			// Returns the conjugate element of this Quaternion.
			[[nodiscard]] constexpr auto Conjugate() const noexcept;

			// Changes this Quaternion to its conjugate element.
			// Returns a reference to this Quaternion.
			LEOPPHAPI Quaternion& ConjugateInPlace();

			// Returns the inverse of this Quaternion.
			[[nodiscard]] LEOPPHAPI Quaternion Inverse() const;

			// Changes this Quaternion to its inverse.
			// Returns a reference to this Quaternion.
			LEOPPHAPI Quaternion& Invert();

			// Creates a 4x4 rotation matrix that represents the same rotation as this Quaternion.
			// It is assumed that this Quaternion has a norm of 1. If not so, it must be normalized beforehand.
			// Quaternions other than unit ones will produce invalid values.
			[[nodiscard]] LEOPPHAPI explicit operator Matrix4() const;

			// Rotates the input vector by this Quaternion.
			// It is assumed that this Quaternion has a norm of 1. If not so, it must be normalized beforehand.
			// Quaternions other than unit ones will produce invalid values.
			template<class T>
			[[nodiscard]] constexpr auto Rotate(internal::Vector<T, 3> const& vec) const noexcept;

		private:
			float m_W;
			float m_X;
			float m_Y;
			float m_Z;
	};


	// Definitions

	constexpr Quaternion::Quaternion(float const w, float const x, float const y, float const z) :
		m_W{w},
		m_X{x},
		m_Y{y},
		m_Z{z}
	{}


	constexpr auto Quaternion::W() const noexcept
	{
		return m_W;
	}


	constexpr auto Quaternion::X() const noexcept
	{
		return m_X;
	}


	constexpr auto Quaternion::Y() const noexcept
	{
		return m_Y;
	}


	constexpr auto Quaternion::Z() const noexcept
	{
		return m_Z;
	}


	constexpr auto Quaternion::W(float const w) noexcept
	{
		m_W = w;
	}


	constexpr auto Quaternion::X(float const x) noexcept
	{
		m_X = x;
	}


	constexpr auto Quaternion::Y(float const y) noexcept
	{
		m_Y = y;
	}


	constexpr auto Quaternion::Z(float const z) noexcept
	{
		m_Z = z;
	}


	constexpr auto Quaternion::Conjugate() const noexcept
	{
		return Quaternion{m_W, -m_X, -m_Y, -m_Z};
	}


	template<class T>
	constexpr auto Quaternion::Rotate(internal::Vector<T, 3> const& vec) const noexcept
	{
		auto const retQuat{*this * Quaternion{0, vec[0], vec[1], vec[2]} * Conjugate()};
		return internal::Vector<T, 3>{retQuat.m_X, retQuat.m_Y, retQuat.m_Z};
	}


	// Returns the Hamilton product of the input Quaternions.
	[[nodiscard]] constexpr auto operator*(Quaternion const& left, Quaternion const& right) noexcept
	{
		return Quaternion
		{
			left.W() * right.W() - left.X() * right.X() - left.Y() * right.Y() - left.Z() * right.Z(),
			left.W() * right.X() + left.X() * right.W() + left.Y() * right.Z() - left.Z() * right.Y(),
			left.W() * right.Y() - left.X() * right.Z() + left.Y() * right.W() + left.Z() * right.X(),
			left.W() * right.Z() + left.X() * right.Y() - left.Y() * right.X() + left.Z() * right.W()
		};
	}


	// Sets the left operand to the Hamilton product of the input Quaternions.
	// Returns a reference to the left operand.
	constexpr auto& operator*=(Quaternion& left, Quaternion const& right) noexcept
	{
		return left = left * right;
	}


	// Prints the input Quaternion on the specified output stream.
	LEOPPHAPI std::ostream& operator<<(std::ostream& os, Quaternion const& q);
}
