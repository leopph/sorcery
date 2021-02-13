#pragma once

#include "leopphapi.h"

#include <type_traits>
#include <ostream>
#include <cstddef>


namespace leopph
{
	template<class T, size_t N>
	class LEOPPHAPI Vector
	{
		static_assert(N >= 2, "Vector dimension must be at least 2!");


	private:
		T m_Data[N];


	public:
		// constructors
		Vector();
		Vector(const T& value);
		Vector(const Vector<T, N>& other);

		template<class... T1, std::enable_if_t<std::conjunction_v<std::is_convertible<T1, T>...> && sizeof...(T1) == N, bool> = false>
		Vector(const T1&... args) :
			m_Data{ static_cast<T>(args)... }
		{}





		// quick access factories
		static Vector<T, N> Up();
		static Vector<T, N> Down();
		static Vector<T, N> Left();
		static Vector<T, N> Right();

		template<size_t N1 = N, std::enable_if_t<N1 >= 3 && N1 == N, bool> = false>
		static Vector<T, N> Forward()
		{
			Vector<T, N> ret;
			ret[2] = 1;
			return ret;
		}

		template<size_t N1 = N, std::enable_if_t<N1 >= 3 && N1 == N, bool> = false>
		static Vector<T, N> Backward()
		{
			Vector<T, N> ret;
			ret[2] = -1;
			return ret;
		}





		// get stored values as pointer
		const T* Data() const;
		T* Data();





		// member operators
		Vector<T, N>& operator=(const Vector<T, N>& other);
		const T& operator[](size_t index) const;
		T& operator[](size_t index);




		// magnitude
		float Length() const;




		// normalization
		Vector<T, N> Normalized() const;
		Vector<T, N>& Normalize();





		// dot product
		static T Dot(const Vector<T, N>& left, const Vector<T, N>& right);




		// cross product, only for 3d vector
		template<size_t N1 = N, std::enable_if_t<N1 == 3 && N1 == N, bool> = false>
		static Vector<T, N> Cross(const Vector<T, N>& left, const Vector<T, N>& right)
		{
			return Vector<T, N> {	left[1] * right[2] - left[2] * right[1],
				left[2] * right[0] - left[0] * right[2],
				left[0] * right[1] - left[1] * right[0] };
		}


		// distance
		static float Distance(const Vector<T, N>& left, const Vector<T, N>& right);
	};






	// non member operators
	template<class T, size_t N>
	Vector<T, N> operator-(const Vector<T, N>& operand)
	{
		Vector<T, N> ret{ operand };

		for (size_t i = 0; i < N; i++)
			ret[i] = -ret[i];

		return ret;
	}




	template<class T, size_t N>
	Vector<T, N> operator+(const Vector<T, N>& left, const Vector<T, N>& right)
	{
		Vector<T, N> ret;

		for (size_t i = 0; i < N; i++)
			ret[i] = left[i] + right[i];

		return ret;
	}

	template<class T, size_t N>
	Vector<T, N> operator+=(Vector<T, N>& left, const Vector<T, N>& right)
	{
		return left = left + right;
	}




	template<class T, size_t N>
	Vector<T, N> operator-(const Vector<T, N>& left, const Vector<T, N>& right)
	{
		return left + -right;
	}

	template<class T, size_t N>
	Vector<T, N> operator-=(Vector<T, N>& left, const Vector<T, N>& right)
	{
		return left = left - right;
	}




	template<class T1, class T2, size_t N, std::enable_if_t<std::is_convertible_v<T2, T1>, bool> = false>
	Vector<T1, N> operator*(const Vector<T1, N>& left, const T2& right)
	{
		Vector<T1, N> ret{ left };

		for (size_t i = 0; i < N; i++)
			ret[i] *= right;

		return ret;
	}

	template<class T1, class T2, size_t N, std::enable_if_t<std::is_convertible_v<T2, T1>, bool> = false>
	Vector<T1, N> operator*(const T2& left, const Vector<T1, N>& right)
	{
		return right * left;
	}

	template<class T1, class T2, size_t N, std::enable_if_t<std::is_convertible_v<T2, T1>, bool> = false>
	Vector<T1, N> operator*=(Vector<T1, N>& left, const T2& right)
	{
		return left = left * right;
	}

	template<class T1, class T2, size_t N, std::enable_if_t<std::is_convertible_v<T2, T1>, bool> = false>
	Vector<T1, N> operator*=(const T2& left, Vector<T1, N>& right)
	{
		return right = left * right;
	}




	template<class T1, class T2, size_t N, std::enable_if_t<std::is_convertible_v<T2, T1>, bool> = false>
	Vector<T1, N> operator/(const Vector<T1, N>& left, const T2& right)
	{
		return left * (static_cast<T1>(1) / right);
	}

	template<class T1, class T2, size_t N, std::enable_if_t<std::is_convertible_v<T2, T1>, bool> = false>
	Vector<T1, N> operator/=(Vector<T1, N>& left, const T2& right)
	{
		return left = left * (static_cast<T1>(1) / right);
	}




	template<class T, size_t N>
	bool operator==(const Vector<T, N>& left, const Vector<T, N>& right)
	{
		for (size_t i = 0; i < N; i++)
			if (left[i] != right[i])
				return false;

		return true;
	}

	template<class T, size_t N>
	bool operator!=(const Vector<T, N>& left, const Vector<T, N>& right)
	{
		return !(left == right);
	}




	template<class T, size_t N>
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



	
	// aliasing
	using Vector4 = Vector<float, 4>;
	using Vector3 = Vector<float, 3>;
	using Vector2 = Vector<float, 2>;
}