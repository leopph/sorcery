#pragma once

#include "../api/LeopphApi.hpp"
#include "Matrix.hpp"
#include "Vector.hpp"

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

		Quaternion& operator=(const Quaternion& other);
		Quaternion& operator=(Quaternion&& other) noexcept;

		/* Get the Quaternion's components.
		 * W is the real part.
		 * X is the coefficient of i.
		 * Y is the coefficient of j.
		 * Z is the coefficient of k. */
		float& W, & X, & Y, & Z;

	    /* Get the Quaternion's components.
		 * Index 0 is the real part.
		 * Index 1 is the coefficient of i.
		 * Index 2 is the coefficient of j.
		 * Index 3 is the coefficient of k. */
		[[nodiscard]]
		const float& operator[](std::size_t index) const;

		/* Get the Quaternion's components.
		 * Index 0 is the real part.
		 * Index 1 is the coefficient of i.
		 * Index 2 is the coefficient of j.
		 * Index 3 is the coefficient of k. */
		[[nodiscard]]
		float& operator[](std::size_t index);

		[[nodiscard]]
		Vector3 EulerAngles() const;

		// Get the norm of the Quaternion.
		[[nodiscard]]
		float Norm() const;

		// Return a Quaternion that represents the same orientation, but has a norm of 1.
		[[nodiscard]]
		Quaternion Normalized() const;

		/* Modifies this Quaternion so that it represents the same orientation, but has a norm of 1.
		 * Returns a reference to this Quaternion. */
		Quaternion& Normalize();

		// Returns the conjugate element of this Quaternion.
		[[nodiscard]]
		Quaternion Conjugate() const;

		/* Changes this Quaternion to its conjugate element.
		 * Returns a reference to this Quaternion. */
		Quaternion& ConjugateInPlace();

		// Returns the inverse of this Quaternion.
		[[nodiscard]]
		Quaternion Inverse() const;

		/* Changes this Quaternion to its inverse.
		 * Returns a reference to this Quaternion. */
		Quaternion& Invert();

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
		impl::Vector<T, 3> Rotate(const impl::Vector<T, 3>& vec) const
		{
			const auto retQuat{*this * Quaternion{0, vec[0], vec[1], vec[2]} *Conjugate()};
			return impl::Vector<T, 3>{retQuat.m_X, retQuat.m_Y, retQuat.m_Z};
		}


	private:
		float m_W;
		float m_X;
		float m_Y;
		float m_Z;
	};


	// Returns the Hamilton product of the input Quaternions.
	LEOPPHAPI Quaternion operator*(const Quaternion& left, const Quaternion& right);

	/* Sets the left operand to the Hamilton product of the input Quaternions.
	 * Returns a reference to the left operand. */
	LEOPPHAPI Quaternion& operator*=(Quaternion& left, const Quaternion& right);

	// Prints the input Quaternion on the specified output stream. */
	LEOPPHAPI std::ostream& operator<<(std::ostream& os, const Quaternion& q);
}