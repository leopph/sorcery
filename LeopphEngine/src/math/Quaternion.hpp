#pragma once

#include "Matrix.hpp"
#include "Vector.hpp"
#include "../api/LeopphApi.hpp"

#include <cstddef>
#include <ostream>


namespace leopph
{
	// Quaternions are a 4 dimensional extension to complex numbers. In LeopphEngine, they are used to calculate rotations.
	class LEOPPHAPI Quaternion
	{
		public:
			// Create a Quaternion by its real and imaginary components.
			explicit Quaternion(float w = 1.0f, float x = 0.0f, float y = 0.0f, float z = 0.0f);

			/* Create a Quaternion that rotates 'angleDegrees' amount around the 'axis' axis.
			 * 'Axis' does not need to be a unit vector.
			 * The Quaternion will have a norm of 1. */
			explicit Quaternion(const Vector3& axis, float angleDegrees);

			Quaternion(const Quaternion& other);
			Quaternion(Quaternion&& other) noexcept;

			~Quaternion() = default;

			auto operator=(const Quaternion& other) -> Quaternion&;
			auto operator=(Quaternion&& other) noexcept -> Quaternion&;

			/* Get the Quaternion's components.
			 * W is the real part.
			 * X is the coefficient of i.
			 * Y is the coefficient of j.
			 * Z is the coefficient of k. */
			float &W, &X, &Y, &Z;

			/* Get the Quaternion's components.
			 * Index 0 is the real part.
			 * Index 1 is the coefficient of i.
			 * Index 2 is the coefficient of j.
			 * Index 3 is the coefficient of k. */
			[[nodiscard]]
			auto operator[](std::size_t index) const -> const float&;

			/* Get the Quaternion's components.
			 * Index 0 is the real part.
			 * Index 1 is the coefficient of i.
			 * Index 2 is the coefficient of j.
			 * Index 3 is the coefficient of k. */
			[[nodiscard]]
			auto operator[](std::size_t index) -> float&;

			[[nodiscard]]
			auto EulerAngles() const -> Vector3;

			// Get the norm of the Quaternion.
			[[nodiscard]]
			auto Norm() const -> float;

			// Return a Quaternion that represents the same orientation, but has a norm of 1.
			[[nodiscard]]
			auto Normalized() const -> Quaternion;

			/* Modifies this Quaternion so that it represents the same orientation, but has a norm of 1.
			 * Returns a reference to this Quaternion. */
			auto Normalize() -> Quaternion&;

			// Returns the conjugate element of this Quaternion.
			[[nodiscard]]
			auto Conjugate() const -> Quaternion;

			/* Changes this Quaternion to its conjugate element.
			 * Returns a reference to this Quaternion. */
			auto ConjugateInPlace() -> Quaternion&;

			// Returns the inverse of this Quaternion.
			[[nodiscard]]
			auto Inverse() const -> Quaternion;

			/* Changes this Quaternion to its inverse.
			 * Returns a reference to this Quaternion. */
			auto Invert() -> Quaternion&;

			/* Creates a 4x4 rotation matrix that represents the same rotation as this Quaternion.
			 * It is assumed that this Quaternion has a norm of 1. If not so, it must be normalized beforehand.
			 * Quaternions other than unit ones will produce invalid values. */
			[[nodiscard]]
			explicit operator Matrix4() const;

			/* Rotates the input vector by this Quaternion.
			 * It is assumed that this Quaternion has a norm of 1. If not so, it must be normalized beforehand.
			 * Quaternions other than unit ones will produce invalid values. */
			template<class T>
			[[nodiscard]]
			auto Rotate(const internal::Vector<T, 3>& vec) const -> internal::Vector<T, 3>
			{
				const auto retQuat{*this * Quaternion{0, vec[0], vec[1], vec[2]} * Conjugate()};
				return internal::Vector<T, 3>{retQuat.m_X, retQuat.m_Y, retQuat.m_Z};
			}

		private:
			float m_W;
			float m_X;
			float m_Y;
			float m_Z;
	};


	// Returns the Hamilton product of the input Quaternions.
	LEOPPHAPI auto operator*(const Quaternion& left, const Quaternion& right) -> Quaternion;

	/* Sets the left operand to the Hamilton product of the input Quaternions.
	 * Returns a reference to the left operand. */
	LEOPPHAPI auto operator*=(Quaternion& left, const Quaternion& right) -> Quaternion&;

	// Prints the input Quaternion on the specified output stream.
	LEOPPHAPI auto operator<<(std::ostream& os, const Quaternion& q) -> std::ostream&;
}
