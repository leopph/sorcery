#include "vector.h"

using std::size_t;

namespace leopph
{
	// CTOR
	template<class T, size_t N>
	Vector<T, N>::Vector() :
		m_Data{}
	{}

	template<class T, size_t N>
	Vector<T, N>::Vector(const T& value) :
		m_Data{}
	{
		for (size_t i = 0; i < N; i++)
			m_Data[i] = value;
	}

	template<class T, size_t N>
	Vector<T, N>::Vector(const Vector<T, N>& other) :
		m_Data{}
	{
		for (size_t i = 0; i < N; i++)
			m_Data[i] = other.m_Data[i];
	}






	// FACTORIES
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







	// DATA ACCESS
	template<class T, size_t N>
	const T* Vector<T, N>::Data() const
	{
		return &m_Data[0];
	}

	template<class T, size_t N>
	T* Vector<T, N>::Data()
	{
		return const_cast<T*>(const_cast<const Vector<T, N>*>(this)->Data());
	}







	// MEMBER OPERATORS
	template<class T, size_t N>
	Vector<T, N>& Vector<T, N>::operator=(const Vector<T, N>& other)
	{
		if (this == &other)
			return *this;

		for (size_t i = 0; i < N; i++)
			m_Data[i] = other.m_Data[i];

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





	// MAGNITUDE
	template<class T, size_t N>
	float Vector<T, N>::Length() const
	{
		float sqrSum{};

		for (size_t i = 0; i < N; i++)
			sqrSum += std::powf(m_Data[i], 2);

		return std::sqrtf(sqrSum);
	}




	// NORMALIZATION
	template<class T, size_t N>
	Vector<T, N> Vector<T, N>::Normalized() const
	{
		return Vector<T, N>{ *this }.Normalize();
	}

	template<class T, size_t N>
	Vector<T, N>& Vector<T, N>::Normalize()
	{
		float length = Length();

		for (size_t i = 0; i < N; i++)
			m_Data[i] /= length;

		return *this;
	}





	// DOT PRODUCT
	template<class T, size_t N>
	T Vector<T, N>::Dot(const Vector<T, N>& left, const Vector<T, N>& right)
	{
		T ret{};

		for (size_t i = 0; i < N; i++)
			ret += left[i] * right[i];

		return ret;
	}






	// VECTOR DISTANCE
	template<class T, size_t N>
	float Vector<T, N>::Distance(const Vector<T, N>& left, const Vector<T, N>& right)
	{
		T sum{};

		for (size_t i = 0; i < N; i++)
			sum += static_cast<T>(std::powf(static_cast<float>(left[i] - right[i]), 2));

		return static_cast<T>(std::sqrt(sum));
	}






	// INSTANCES
	template class LEOPPHAPI Vector<float, 4>;
	template class LEOPPHAPI Vector<float, 3>;
	template class LEOPPHAPI Vector<float, 2>;
}