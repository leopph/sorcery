#include "matrix.h"

namespace leopph
{
	// CTOR
	template<class T, size_t N, size_t M>
	Matrix<T, N, M>::Matrix() :
		m_Data{}
	{}

	template<class T, size_t N, size_t M>
	Matrix<T, N, M>::Matrix(const T& value) :
		m_Data{}
	{
		for (size_t i = 0; i < N && i < M; i++)
			m_Data[i][i] = value;
	}

	template<class T, size_t N, size_t M>
	Matrix<T, N, M>::Matrix(const Matrix<T, N, M>& other) :
		m_Data{}
	{
		for (size_t i = 0; i < N; i++)
			m_Data[i] = other.m_Data[i];
	}




	// DATA ACCESS
	template<class T, size_t N, size_t M>
	const T* Matrix<T, N, M>::Data() const
	{
		return m_Data[0].Data();
	}

	template<class T, size_t N, size_t M>
	T* Matrix<T, N, M>::Data()
	{
		return const_cast<T*>(const_cast<const Matrix<T, N, M>*>(this)->Data());
	}




	// MEMBER OPERATORS
	template<class T, size_t N, size_t M>
	Matrix<T, N, M>& Matrix<T, N, M>::operator=(const Matrix<T, N, M>& other)
	{
		if (this == &other)
			return *this;

		for (size_t i = 0; i < N; i++)
			for (size_t j = 0; j < M; j++)
				m_Data[i][j] = other.m_Data[i][j];

		return *this;
	}

	template<class T, size_t N, size_t M>
	const Vector<T, M>& Matrix<T, N, M>::operator[](size_t index) const
	{
		return m_Data[index];
	}

	template<class T, size_t N, size_t M>
	Vector<T, M>& Matrix<T, N, M>::operator[](size_t index)
	{
		return const_cast<Vector<T, M>&>(const_cast<const Matrix<T, N, M>*>(this)->operator[](index));
	}





	// TRANSPOSE
	template<class T, size_t N, size_t M>
	Matrix<T, M, N> Matrix<T, N, M>::Transposed() const
	{
		Matrix<T, M, N> ret;

		for (size_t i = 0; i < N; i++)
			for (size_t j = 0; j < M; j++)
				ret[j][i] = m_Data[i][j];

		return ret;
	}






	// INSTANCES
	template class LEOPPHAPI Matrix<float, 2, 2>;
	template class LEOPPHAPI Matrix<float, 3, 3>;
	template class LEOPPHAPI Matrix<float, 4, 4>;
}