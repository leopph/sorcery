#pragma once

#include "../api/leopphapi.h"
#include "Matrix.hpp"
#include "Vector.hpp"

#include <cstddef>
#include <ostream>


namespace leopph
{
	/*---------------------------------------------------------------------------------
	The Quaternion class provides several ways to aid in solving mathematical problems.
	It is mainly used to represent rotations internally.
	See "transform.h" for additional information.
	---------------------------------------------------------------------------------*/

	class LEOPPHAPI Quaternion
	{
	public:
		/* Initialize the Quaternion directly with its components' values.
		It is recommended to use different constructors instead. */
		explicit Quaternion(float w = 1.0f, float x = 0.0f, float y = 0.0f, float z = 0.0f);

		/* Create a Quaternion that rotates "angle" amount around the "axis" axis */
		Quaternion(const Vector3& axis, float angleDegrees);

		Quaternion(const Quaternion& other) = default;
		Quaternion(Quaternion&& other) = default;
		Quaternion& operator=(const Quaternion& other) = default;
		Quaternion& operator=(Quaternion&& other) = default;
		~Quaternion() = default;

		[[nodiscard]] Vector3 EulerAngles() const;

		/* Get the individual elements using an index.
		* 0 - W component
		* 1 - X component
		* 2 - Y component
		* 3 - Z component */
		const float& operator[](std::size_t index) const;
		float& operator[](std::size_t index);

		/* Mathematical magnitude of the Quaternion */
		[[nodiscard]] float Magnitude() const;

		/* Cast the Quaternion to a 3D rotation Matrix */
		explicit operator Matrix4() const;

	private:
		float w{ 1.0f };
		float x{ 0.0f };
		float y{ 0.0f };
		float z{ 0.0f };
	};


	/*----------------
	Standard operators
	----------------*/

	LEOPPHAPI Quaternion operator*(const Quaternion& left, const Quaternion& right);
	LEOPPHAPI Quaternion& operator*=(Quaternion& left, const Quaternion& right);
	LEOPPHAPI std::ostream& operator<<(std::ostream& os, const Quaternion& q);
}