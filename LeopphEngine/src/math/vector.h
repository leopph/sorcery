#pragma once

#include <type_traits>
#include <ostream>
#include <cstddef>
#include <concepts>


namespace leopph
{
	namespace impl
	{
		/*------------------------------------------------------------------------------------------
		The Vector class template provides several ways to aid in solving linear algebraic problems.
		Vector are row/column agnostic, they are always interpreted in the form they have to be
		in order for formulas to make sense.
		DO NOT INSTANTIATE THIS TEMPLATE EXPLICITLY UNLESS NECESSARY!
		There are several predefined implementations at the bottom of this file.
		------------------------------------------------------------------------------------------*/

		template<class T, std::size_t N> requires(N > 1)
		class Vector
		{
		private:
			T m_Data[N];


		public:
			/* Zero Constructor */
			Vector() :
				m_Data{}
			{}

			/* Fill Constructor */
			Vector(const T& value) :
				m_Data{}
			{
				for (size_t i = 0; i < N; i++)
					m_Data[i] = value;
			}

			/* Copy Constructor */
			Vector(const Vector<T, N>& other) :
				m_Data{}
			{
				for (size_t i = 0; i < N; i++)
					m_Data[i] = other.m_Data[i];
			}

			/* All Elements Constructor */
			template<std::convertible_to<T>... T1>
			Vector(const T1&... args) requires(sizeof...(T1) == N) :
				m_Data{ static_cast<T>(args)... }
			{}





			/* Quick Access Factories */
			static Vector<T, N> Up()
			{
				Vector<T, N> ret;
				ret[1] = 1;
				return ret;
			}
			static Vector<T, N> Down()
			{
				Vector<T, N> ret;
				ret[1] = -1;
				return ret;
			}
			static Vector<T, N> Left()
			{
				Vector<T, N> ret;
				ret[0] = -1;
				return ret;
			}
			static Vector<T, N> Right()
			{
				Vector<T, N> ret;
				ret[0] = 1;
				return ret;
			}
			static Vector<T, N> Forward() requires(N >= 3)
			{
				Vector<T, N> ret;
				ret[2] = 1;
				return ret;
			}
			static Vector<T, N> Backward() requires(N >= 3)
			{
				Vector<T, N> ret;
				ret[2] = -1;
				return ret;
			}





			/* Returns a pointer to the internal data structure.
			DO NOT USE THIS UNLESS NECESSARY! */
			const T* Data() const
			{
				return &m_Data[0];
			}
			T* Data()
			{
				return const_cast<T*>(const_cast<const Vector<T, N>*>(this)->Data());
			}





			/* Copy Assignment */
			Vector<T, N>& operator=(const Vector<T, N>& other)
			{
				if (this == &other)
					return *this;

				for (size_t i = 0; i < N; i++)
					m_Data[i] = other.m_Data[i];

				return *this;
			}


			/* Returns the Nth component of the vector */
			const T& operator[](size_t index) const
			{
				return m_Data[index];
			}
			T& operator[](size_t index)
			{
				return const_cast<T&>(const_cast<const Vector<T, N>*>(this)->operator[](index));
			}




			/* Mathematical vector magnitude */
			float Length() const
			{
				float sqrSum{};

				for (size_t i = 0; i < N; i++)
					sqrSum += std::powf(m_Data[i], 2);

				return std::sqrtf(sqrSum);
			}




			/* Normalization changes the vector in-place to have a magnitude of 1,
			but point in the same direction */
			Vector<T, N>& Normalize()
			{
				float length = Length();

				for (size_t i = 0; i < N; i++)
					m_Data[i] /= length;

				return *this;
			}

			/* Returns a new vector that has a magnitude of 1 and points in the same direction */
			Vector<T, N> Normalized() const
			{
				return Vector<T, N>{ *this }.Normalize();
			}





			/* Mathematical dot product of two equal dimension vectors */
			static T Dot(const Vector<T, N>& left, const Vector<T, N>& right)
			{
				T ret{};

				for (size_t i = 0; i < N; i++)
					ret += left[i] * right[i];

				return ret;
			}




			/* Mathematical cross product, only between 3D vectors */
			static Vector<T, N> Cross(const Vector<T, N>& left, const Vector<T, N>& right) requires(N == 3)
			{
				return Vector<T, N> {	left[1] * right[2] - left[2] * right[1],
					left[2] * right[0] - left[0] * right[2],
					left[0] * right[1] - left[1] * right[0] };
			}


			/* Mathematical vector distance */
			static float Distance(const Vector<T, N>& left, const Vector<T, N>& right)
			{
				T sum{};

				for (size_t i = 0; i < N; i++)
					sum += static_cast<T>(std::powf(static_cast<float>(left[i] - right[i]), 2));

				return static_cast<T>(std::sqrt(sum));
			}
		};





		/*-----------------------------------
		Other standard mathematical operators
		-----------------------------------*/
		
		template<class T, std::size_t N>
		Vector<T, N> operator-(const Vector<T, N>& operand)
		{
			Vector<T, N> ret{ operand };

			for (size_t i = 0; i < N; i++)
				ret[i] = -ret[i];

			return ret;
		}




		template<class T, std::size_t N>
		Vector<T, N> operator+(const Vector<T, N>& left, const Vector<T, N>& right)
		{
			Vector<T, N> ret;

			for (size_t i = 0; i < N; i++)
				ret[i] = left[i] + right[i];

			return ret;
		}

		template<class T, std::size_t N>
		Vector<T, N> operator+=(Vector<T, N>& left, const Vector<T, N>& right)
		{
			return left = left + right;
		}




		template<class T, std::size_t N>
		Vector<T, N> operator-(const Vector<T, N>& left, const Vector<T, N>& right)
		{
			return left + -right;
		}

		template<class T, std::size_t N>
		Vector<T, N> operator-=(Vector<T, N>& left, const Vector<T, N>& right)
		{
			return left = left - right;
		}




		template<class T1, std::convertible_to<T1> T2, std::size_t N>
		Vector<T1, N> operator*(const Vector<T1, N>& left, const T2& right)
		{
			Vector<T1, N> ret{ left };

			for (size_t i = 0; i < N; i++)
				ret[i] *= right;

			return ret;
		}

		template<class T1, std::convertible_to<T1> T2, std::size_t N>
		Vector<T1, N> operator*(const T2& left, const Vector<T1, N>& right)
		{
			return right * left;
		}

		template<class T1, std::convertible_to<T1> T2, std::size_t N>
		Vector<T1, N> operator*=(Vector<T1, N>& left, const T2& right)
		{
			return left = left * right;
		}

		template<class T1, std::convertible_to<T1> T2, std::size_t N>
		Vector<T1, N> operator*=(const T2& left, Vector<T1, N>& right)
		{
			return right = left * right;
		}




		template<class T1, std::convertible_to<T1> T2, std::size_t N>
		Vector<T1, N> operator/(const Vector<T1, N>& left, const T2& right)
		{
			return left * (static_cast<T1>(1) / right);
		}

		template<class T1, std::convertible_to<T1> T2, std::size_t N>
		Vector<T1, N> operator/=(Vector<T1, N>& left, const T2& right)
		{
			return left = left * (static_cast<T1>(1) / right);
		}




		template<class T, std::size_t N>
		bool operator==(const Vector<T, N>& left, const Vector<T, N>& right)
		{
			for (size_t i = 0; i < N; i++)
				if (left[i] != right[i])
					return false;

			return true;
		}

		template<class T, std::size_t N>
		bool operator!=(const Vector<T, N>& left, const Vector<T, N>& right)
		{
			return !(left == right);
		}




		template<class T, std::size_t N>
		std::ostream& operator<<(std::ostream& stream, const Vector<T, N>& vector)
		{
			stream << "(";

			for (size_t i = 0; i < N; i++)
			{
				stream << vector[i];

				if (i != N - 1)
					stream << ", ";
			}

			stream << ")";

			return stream;
		}
	}




	/*------------------------------------------------------
	Use these instances where you can in your business logic
	to get the best compatibility and performance.
	------------------------------------------------------*/

	using Vector4 = impl::Vector<float, 4>;
	using Vector3 = impl::Vector<float, 3>;
	using Vector2 = impl::Vector<float, 2>;
}