#pragma once

#include "LeopphMath.hpp"

#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <numeric>
#include <ostream>


namespace leopph
{
	namespace internal
	{
		/* The Vector class template provides a way to represent N dimensional vectors over a T type.
		 * Vectors are row/column agnostic. They are always interpreted in the necessary form for the specific formulas.
		 * This template may be explicitly instantiated, but LeopphEngine provides several predefined instantiations at the bottom of this file. */
		template<class T, std::size_t N>
			requires(N > 1)
		class Vector
		{
		public:
			// Creates a Vector with all components set to 0.
			Vector() :
				m_Data{}
			{}

			// Creates a Vector with all components set to the input value.
			explicit Vector(const T& value)
			{
				m_Data.fill(value);
			}

			// Creates a Vector with components set to the input values.
			template<std::convertible_to<T>... T1>
			explicit Vector(const T1&... args)
				requires(sizeof...(T1) == N) :
				m_Data{static_cast<T>(args)...}
			{}

			// Creates an N-1 dimensional Vector that is the same as the input Vector without its last component.
			explicit Vector(const Vector<T, N + 1>& other)
			{
				std::ranges::copy_n(other.Data().begin(), N, m_Data.begin());
			}

			Vector(const Vector<T, N>& other) = default;
			Vector(Vector<T, N>&& other) = default;

			~Vector() = default;

			Vector<T, N>& operator=(const Vector<T, N>& other) = default;
			Vector<T, N>& operator=(Vector<T, N>&& other) = default;

			// Creates a Vector with its second component set to 1 and all other components set to 0.
			static Vector<T, N> Up()
			{
				Vector<T, N> ret;
				ret[1] = 1;
				return ret;
			}

			// Creates a Vector with its second component set to -1 and all other components set to 0.
			static Vector<T, N> Down()
			{
				Vector<T, N> ret;
				ret[1] = -1;
				return ret;
			}

			// Creates a Vector with its first component set to -1 and all other components set to 0.
			static Vector<T, N> Left()
			{
				Vector<T, N> ret;
				ret[0] = -1;
				return ret;
			}

			// Creates a Vector with its first component set to 1 and all other components set to 0.
			static Vector<T, N> Right()
			{
				Vector<T, N> ret;
				ret[0] = 1;
				return ret;
			}

			// Creates a Vector with its third component set to 1 and all other components set to 0.
			static Vector<T, N> Forward()
				requires(N >= 3)
			{
				Vector<T, N> ret;
				ret[2] = 1;
				return ret;
			}

			// Creates a Vector with its third component set to -1 and all other components set to 0.
			static Vector<T, N> Backward()
				requires(N >= 3)
			{
				Vector<T, N> ret;
				ret[2] = -1;
				return ret;
			}

			// Returns a reference to the internal data structure.
			[[nodiscard]] const std::array<T, N>& Data() const
			{
				return m_Data;
			}

			/* Get the Vector's components.
			 * Indexes are not bounds-checked. */
			const T& operator[](size_t index) const
			{
				return m_Data[index];
			}

			/* Get the Vector's components.
			 * Indexes are not bounds-checked. */
			T& operator[](const size_t index)
			{
				return const_cast<T&>(const_cast<const Vector<T, N>*>(this)->operator[](index));
			}

			// Get the length of this Vector.
			[[nodiscard]] float Length() const
			{
				return math::Sqrt(
					static_cast<float>(
						std::accumulate(m_Data.begin(), m_Data.end(), static_cast<T>(0), [](const T& sum, const T& elem)
				{
					return sum + math::Pow(elem, 2);
				})));
			}

			// Returns a Vector that has the same direction as this Vector, but has a length of 1.
			[[nodiscard]] Vector<T, N> Normalized() const
			{
				return Vector<T, N>{*this}.Normalize();
			}

			/* Changes this Vector so that it has the same direction, but a length of 1.
			 * Returns a reference to this Vector. */
			Vector<T, N>& Normalize()
			{
				auto length = Length();

				if (length != 0)
				{
					std::for_each(m_Data.begin(), m_Data.end(), [length](T& elem)
					{
						elem /= static_cast<T>(length);
					});
				}

				return *this;
			}

	        // Creates a Vector that is the copy of this Vector, extended with an additional component with a value of 1.
			explicit operator Vector<T, N + 1>() const
			{
				Vector<T, N + 1> ret;

				for (std::size_t i = 0; i < N; i++)
				{
					ret[i] = m_Data[i];
				}

				ret[N] = static_cast<T>(1);
				return ret;
			}

			// Returns the dot product of the input Vectors.
			static T Dot(const Vector<T, N>& left, const Vector<T, N>& right)
			{
				T ret{};

				for (size_t i = 0; i < N; i++)
				{
					ret += left[i] * right[i];
				}

				return ret;
			}

			// Returns the cross product of the input Vectors.
			static Vector<T, N> Cross(const Vector<T, N>& left, const Vector<T, N>& right)
				requires(N == 3)
			{
				return Vector<T, N>
				{
					left[1] * right[2] - left[2] * right[1],
						left[2] * right[0] - left[0] * right[2],
						left[0] * right[1] - left[1] * right[0]
				};
			}

			// Returns the Euclidean distance of the input Vectors.
			static float Distance(const Vector<T, N>& left, const Vector<T, N>& right)
			{
				T sum{};

				for (size_t i = 0; i < N; i++)
				{
					sum += static_cast<T>(math::Pow(static_cast<float>(left[i] - right[i]), 2));
				}

				return static_cast<T>(math::Sqrt(sum));
			}


		private:
			std::array<T, N> m_Data;
		};


		// Returns a Vector that's components are the additives inverses of this Vector's components.
		template<class T, std::size_t N>
		Vector<T, N> operator-(const Vector<T, N>& operand)
		{
			Vector<T, N> ret;
			for (size_t i = 0; i < N; i++)
			{
				ret[i] = -(operand[i]);
			}
			return ret;
		}

		// Returns the sum of the input Vectors.
		template<class T, std::size_t N>
		Vector<T, N> operator+(const Vector<T, N>& left, const Vector<T, N>& right)
		{
			Vector<T, N> ret;
			for (size_t i = 0; i < N; i++)
			{
				ret[i] = left[i] + right[i];
			}
			return ret;
		}

		/* Sets the left operand to the sum of the input Vectors.
		 * Returns a reference to the left operand. */
		template<class T, std::size_t N>
		Vector<T, N>& operator+=(Vector<T, N>& left, const Vector<T, N>& right)
		{
			for (std::size_t i = 0; i < N; i++)
			{
				left[i] += right[i];
			}
			return left;
		}

		// Returns the difference of the input Vectors.
		template<class T, std::size_t N>
		Vector<T, N> operator-(const Vector<T, N>& left, const Vector<T, N>& right)
		{
			Vector<T, N> ret;
			for (std::size_t i = 0; i < N; i++)
			{
				ret[i] = left[i] - right[i];
			}
			return ret;
		}

		/* Sets the left operand to the difference of the input Vectors.
		 * Returns a reference to the left operand. */
		template<class T, std::size_t N>
		Vector<T, N>& operator-=(Vector<T, N>& left, const Vector<T, N>& right)
		{
			for (std::size_t i = 0; i < N; i++)
			{
				left[i] -= right[i];
			}
			return left;
		}

		// Returns the result of the scalar multiplication of the input values.
		template<class T1, std::convertible_to<T1> T2, std::size_t N>
		Vector<T1, N> operator*(const Vector<T1, N>& left, const T2& right)
		{
			Vector<T1, N> ret;
			for (size_t i = 0; i < N; i++)
			{
				ret[i] = left[i] * right;
			}
			return ret;
		}

		// Returns the result of the scalar multiplication of the input values.
		template<class T1, std::convertible_to<T1> T2, std::size_t N>
		Vector<T1, N> operator*(const T2& left, const Vector<T1, N>& right)
		{
			Vector<T1, N> ret;
			for (std::size_t i = 0; i < N; i++)
			{
				ret[i] = left * right[i];
			}
			return ret;
		}

		// Returns the component-wise product of the input Vectors.
		template<class T, std::size_t N>
		Vector<T, N> operator*(const Vector<T, N>& left, const Vector<T, N>& right)
		{
			Vector<T, N> ret;
			for (std::size_t i = 0; i < N; i++)
			{
				ret[i] = left[i] * right[i];
			}
			return ret;
		}

		/* Sets the left operand to the result of the scalar multiplication of the input values.
		 * Returns a reference to the left operand. */
		template<class T1, std::convertible_to<T1> T2, std::size_t N>
		Vector<T1, N>& operator*=(Vector<T1, N>& left, const T2& right)
		{
			for (std::size_t i = 0; i < N; i++)
			{
				left[i] *= right;
			}
			return left;
		}

		/* Sets the left oparend to the component-wise product of the input Vectors.
		 * Returns a reference to the left operand. */
		template<class T, std::size_t N>
		Vector<T, N>& operator*=(Vector<T, N>& left, const Vector<T, N>& right)
		{
			for (std::size_t i = 0; i < N; i++)
			{
				left[i] *= right[i];
			}
			return left;
		}

		// Returns the result of the scalar division of the input values.
		template<class T1, std::convertible_to<T1> T2, std::size_t N>
		Vector<T1, N> operator/(const Vector<T1, N>& left, const T2& right)
		{
			Vector<T1, N> ret;
			for (std::size_t i = 0; i < N; i++)
			{
				ret[i] = left[i] / right;
			}
			return ret;
		}

		// Returns the component-wise quotient of the input Vectors.
		template<class T, std::size_t N>
		Vector<T, N> operator/(const Vector<T, N>& left, const Vector<T, N>& right)
		{
			Vector<T, N> ret;
			for (std::size_t i = 0; i < N; i++)
			{
				ret[i] = left[i] / right[i];
			}
			return ret;
		}

		/* Sets the left operand to the result of the scalar division of the input values.
		 * Returns a reference to the left operand. */
		template<class T1, std::convertible_to<T1> T2, std::size_t N>
		Vector<T1, N>& operator/=(Vector<T1, N>& left, const T2& right)
		{
			for (std::size_t i = 0; i < N; i++)
			{
				left[i] /= right;
			}
			return left;
		}

		/* Sets the left operand to the component-wise quotient of the input Vectors.
		 * Returns a reference to the left operand. */
		template<class T, std::size_t N>
		Vector<T, N>& operator/=(Vector<T, N>& left, const Vector<T, N>& right)
		{
			for (std::size_t i = 0; i < N; i++)
			{
				left[i] /= right[i];
			}
			return left;
		}

		// Returns whether the input Vectors are equal.
		template<class T, std::size_t N>
		bool operator==(const Vector<T, N>& left, const Vector<T, N>& right)
		{
			for (size_t i = 0; i < N; i++)
			{
				if (left[i] != right[i])
				{
					return false;
				}
			}

			return true;
		}

		// Returns whether the input Vectors are not equal.
		template<class T, std::size_t N>
		bool operator!=(const Vector<T, N>& left, const Vector<T, N>& right)
		{
			return !(left == right);
		}

		// Prints the input Vector on the specified output stream.
		template<class T, std::size_t N>
		std::ostream& operator<<(std::ostream& stream, const Vector<T, N>& vector)
		{
			stream << "(";

			for (size_t i = 0; i < N; i++)
			{
				stream << vector[i];

				if (i != N - 1)
				{
					stream << ", ";
				}
			}

			stream << ")";

			return stream;
		}
	}


	// 4D single-precision floating-point Vector.
	using Vector4 = internal::Vector<float, 4>;

	// 3D single-precision floating-point Vector.
	using Vector3 = internal::Vector<float, 3>;

	// 2D single-precision floating-point Vector.
	using Vector2 = internal::Vector<float, 2>;
}
