#include <cmath>
#include <ostream>

#include "vector.h"

namespace leopph
{
	// constructors
	template<class T, size_t N>
	Vector<T, N>::Vector() :
		m_Data{ new T[N]{} }
	{}

	template<class T, size_t N>
	Vector<T, N>::Vector(const T& value) :
		m_Data{ new T[N]{} }
	{
		for (size_t i = 0; i < N; i++)
			m_Data[i] = value;
	}

	template<class T, size_t N>
	template<class... T1, std::enable_if_t<std::conjunction_v<std::is_convertible<T1, T>...> && sizeof...(T1) == N, bool>>
	Vector<T, N>::Vector(const T1&... args) :
		m_Data{ new T[N]{ static_cast<T>(args)... } }
	{}

	template<class T, size_t N>
	Vector<T, N>::Vector(const Vector<T, N>& other) :
		m_Data{ new T[N] }
	{
		for (size_t i = 0; i < N; i++)
			m_Data[i] = other.m_Data[i];
	}




	// destructor
	template<class T, size_t N>
	Vector<T, N>::~Vector()
	{
		delete m_Data;
	}





	// quick access factories
	template<class T, size_t N>
	Vector<T, N> Vector<T, N>::Up()
	{
		Vector<T, N> ret;
		ret[1] = 1;
		return ret;
	}

	template<class T, size_t N>
	Vector<T, N> Vector<T, N>::Down()
	{
		Vector<T, N> ret;
		ret[1] = -1;
		return ret;
	}

	template<class T, size_t N>
	Vector<T, N> Vector<T, N>::Left()
	{
		Vector<T, N> ret;
		ret[0] = -1;
		return ret;
	}

	template<class T, size_t N>
	Vector<T, N> Vector<T, N>::Right()
	{
		Vector<T, N> ret;
		ret[0] = 1;
		return ret;
	}

	template<class T, size_t N>
	template<size_t N1, std::enable_if_t<N1 >= 3 && N1 == N, bool>>
	Vector<T, N> Vector<T, N>::Forward()
	{
		Vector<T, N> ret;
		ret[2] = 1;
		return ret;
	}

	template<class T, size_t N>
	template<size_t N1, std::enable_if_t<N1 >= 3 && N1 == N, bool>>
	Vector<T, N> Vector<T, N>::Backward()
	{
		Vector<T, N> ret;
		ret[2] = -1;
		return ret;
	}



	
	// member operators
	template<class T, size_t N>
	Vector<T, N>& Vector<T, N>::operator=(const Vector<T, N>& other)
	{
		if (this == &other)
			return *this;

		delete[] m_Data;
		m_Data = other.m_Data;

		return *this;
	}

	template<class T, size_t N>
	const T& Vector<T, N>::operator[](size_t index) const
	{
		return m_Data[index];
	}

	template<class T, size_t N>
	T& Vector<T, N>::operator[](size_t index)
	{
		return const_cast<T&>(const_cast<const Vector<T, N>*>(this)->operator[](index));
	}




	// magnitude
	template<class T, size_t N>
	float Vector<T, N>::Length() const
	{
		float sqrSum{};

		for (size_t i = 0; i < N; i++)
			sqrSum += std::powf(m_Data[i], 2);

		return std::sqrtf(sqrSum);
	}



	// normalized copy
	template<class T, size_t N>
	Vector<T, N> Vector<T, N>::Normalized() const
	{
		float length = Length();
		Vector<T, N> ret{ *this };

		for (size_t i = 0; i < N; i++)
			ret[i] /= length;

		return ret;
	}

	// in place normalize
	template<class T, size_t N>
	void Vector<T, N>::Normalize()
	{
		float length = Length();

		for (size_t i = 0; i < N; i++)
			m_Data[i] /= length;
	}





	// dot product
	template<class T, size_t N>
	T Vector<T, N>::Dot(const Vector<T, N>& left, const Vector<T, N>& right)
	{
		T ret{};

		for (size_t i = 0; i < N; i++)
			ret += left[i] * right[i];

		return ret;
	}

	// cross product, only for 3d vector
	template<class T, size_t N>
	template<size_t N1, std::enable_if_t<N1 == 3 && N1 == N, bool>>
	Vector<T, N> Vector<T, N>::Cross(const Vector<T, N>& left, const Vector<T, N>& right)
	{
		return Vector<T, N> { left[1] * right[2] - left[2] * right[1],
			left[2] * right[0] - left[0] * right[2],
			left[0] * right[1] - left[1] * right[0] };
	}








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