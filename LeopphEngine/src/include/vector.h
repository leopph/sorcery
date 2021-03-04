#pragma once

#include <type_traits>
#include <ostream>
#include <cstddef>
#include <concepts>


namespace leopph
{
	namespace implementation
	{
		template<class T, std::size_t N> requires(N > 1)
		class Vector
		{
		private:
			T m_Data[N];


		public:
			// constructors
			Vector() :
				m_Data{}
			{}

			Vector(const T& value) :
				m_Data{}
			{
				for (size_t i = 0; i < N; i++)
					m_Data[i] = value;
			}

			Vector(const Vector<T, N>& other) :
				m_Data{}
			{
				for (size_t i = 0; i < N; i++)
					m_Data[i] = other.m_Data[i];
			}

			template<std::convertible_to<T>... T1>
			Vector(const T1&... args) requires(sizeof...(T1) == N) :
				m_Data{ static_cast<T>(args)... }
			{}





			// quick access factories
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





			// get stored values as pointer
			const T* Data() const
			{
				return &m_Data[0];
			}
			T* Data()
			{
				return const_cast<T*>(const_cast<const Vector<T, N>*>(this)->Data());
			}





			// member operators
			Vector<T, N>& operator=(const Vector<T, N>& other)
			{
				if (this == &other)
					return *this;

				for (size_t i = 0; i < N; i++)
					m_Data[i] = other.m_Data[i];

				return *this;
			}

			const T& operator[](size_t index) const
			{
				return m_Data[index];
			}
			T& operator[](size_t index)
			{
				return const_cast<T&>(const_cast<const Vector<T, N>*>(this)->operator[](index));
			}




			// magnitude
			float Length() const
			{
				float sqrSum{};

				for (size_t i = 0; i < N; i++)
					sqrSum += std::powf(m_Data[i], 2);

				return std::sqrtf(sqrSum);
			}




			// normalization
			Vector<T, N>& Normalize()
			{
				float length = Length();

				for (size_t i = 0; i < N; i++)
					m_Data[i] /= length;

				return *this;
			}
			Vector<T, N> Normalized() const
			{
				return Vector<T, N>{ *this }.Normalize();
			}





			// dot product
			static T Dot(const Vector<T, N>& left, const Vector<T, N>& right)
			{
				T ret{};

				for (size_t i = 0; i < N; i++)
					ret += left[i] * right[i];

				return ret;
			}




			// cross product, only for 3d vector
			static Vector<T, N> Cross(const Vector<T, N>& left, const Vector<T, N>& right) requires(N == 3)
			{
				return Vector<T, N> {	left[1] * right[2] - left[2] * right[1],
					left[2] * right[0] - left[0] * right[2],
					left[0] * right[1] - left[1] * right[0] };
			}


			// distance
			static float Distance(const Vector<T, N>& left, const Vector<T, N>& right)
			{
				T sum{};

				for (size_t i = 0; i < N; i++)
					sum += static_cast<T>(std::powf(static_cast<float>(left[i] - right[i]), 2));

				return static_cast<T>(std::sqrt(sum));
			}
		};






		// non member operators
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




	// aliasing
	using Vector4 = implementation::Vector<float, 4>;
	using Vector3 = implementation::Vector<float, 3>;
	using Vector2 = implementation::Vector<float, 2>;
}