#include <cmath>

#include "vector.h"

namespace leopph
{
	// non member operators
	template<class T, size_t N>
	Vector<T, N> operator-(const Vector<T, N>& operand)
	{
		Vector<T, N> ret{ *this };

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



	template<class T1, class T2, size_t N, std::enable_if_t<std::is_convertible_v<T2, T1>, bool>>
	Vector<T1, N> operator*(const Vector<T1, N>& left, const T2& right)
	{
		Vector<T1, N> ret{ left };

		for (size_t i = 0; i < N; i++)
			ret[i] *= right;

		return ret;
	}

	template<class T1, class T2, size_t N, std::enable_if_t<std::is_convertible_v<T2, T1>, bool>>
	Vector<T1, N> operator*(const T2& left, const Vector<T1, N>& right)
	{
		return right * left;
	}

	template<class T1, class T2, size_t N, std::enable_if_t<std::is_convertible_v<T2, T1>, bool>>
	Vector<T1, N> operator*=(Vector<T1, N>& left, const T2& right)
	{
		return left = left * right;
	}

	template<class T1, class T2, size_t N, std::enable_if_t<std::is_convertible_v<T2, T1>, bool>>
	Vector<T1, N> operator*=(const T2& left, Vector<T1, N>& right)
	{
		return right = left * right;
	}




	template<class T1, class T2, size_t N, std::enable_if_t<std::is_convertible_v<T2, T1>, bool>>
	Vector<T1, N> operator/(const Vector<T1, N>& left, const T2& right)
	{
		return left * (static_cast<T1>(1) / right);
	}

	template<class T1, class T2, size_t N, std::enable_if_t<std::is_convertible_v<T2, T1>, bool>>
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
}