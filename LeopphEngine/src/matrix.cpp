#include <ostream>

#include "matrix.h"


namespace leopph
{
	// constructors
	template<class T, size_t N, size_t M>
	Matrix<T, N, M>::Matrix() :
		m_Data{ new Vector<T, M>[N] }
	{}

	template<class T, size_t N, size_t M>
	Matrix<T, N, M>::Matrix(const T& value) :
		m_Data{ new Vector<T, M>[N] }
	{
		for (size_t i = 0; i < N && i < M; i++)
			m_Data[i][i] = value;
	}

	template<class T, size_t N, size_t M>
	template<class... T1, std::enable_if_t<std::conjunction_v<std::is_convertible<T1, T>...> && sizeof...(T1) == (N > M ? M : N), bool>>
	Matrix<T, N, M>::Matrix(const T1&... args) :
		m_Data{ new Vector<T, M>[N] }
	{
		// TODO think this over man

		T argArr[M > N ? N : M]{ static_cast<T>(args)... };

		for (size_t i = 0; i < M && i < N; i++)
			m_Data[i][i] = argArr[i];
	}

	template<class T, size_t N, size_t M>
	template<class... T1, std::enable_if_t<std::conjunction_v<std::is_convertible<T1, T>...> && sizeof...(T1) == (N * M), bool>>
	Matrix<T, N, M>::Matrix(const T1&... args) :
		m_Data{ new Vector<T, M>[N] }
	{
		// TODO rework candidate

		T argArr[M * N]{ static_cast<T>(args)... };

		for (size_t i = 0; i < N; i++)
			for (size_t j = 0; j < M; j++)
				m_Data[i][j] = argArr[i * M + j];
	}



	// factories
	template<class T, size_t N, size_t M>
	template<size_t N1, size_t M1, std::enable_if_t<N1 == M1 && N1 == N && M1 == M, bool>>
	Matrix<T, N, M> Matrix<T, N, M>::Identity()
	{
		return Matrix<T, N, M>{1};
	}


	// destructor
	template<class T, size_t N, size_t M>
	Matrix<T, N, M>::~Matrix()
	{
		delete[] m_Data;
	}





	// member operators
	template<class T, size_t N, size_t M>
	Matrix<T, N, M>& Matrix<T, N, M>::operator=(const Matrix<T, N, M>& other)
	{
		if (this == &other)
			return *this;

		delete m_Data;
		m_Data = other.m_Data;
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





	// determinant
	template<class T, size_t N, size_t M>
	template<size_t N1, size_t M1, std::enable_if_t<N1 == M1 && N1 == N && M1 == M, bool>>
	float Matrix<T, N, M>::Det() const
	{
		Matrix<T, N, M> tmp{ *this };

		for (size_t i = 1; i < N; i++)
			for (size_t j = 0; j < N; j++)
				tmp[j][0] += tmp[j][i];

		for (size_t i = 1; i < N; i++)
			tmp[i] -= tmp[0];

		float ret{ 1.0f };

		for (size_t i = 0; i < N; i++)
			ret *= tmp[i][i];

		return ret;
	}





	// transposed copy
	template<class T, size_t N, size_t M>
	Matrix<T, M, N> Matrix<T, N, M>::Transposed() const
	{
		Matrix<T, M, N> ret;

		for (size_t i = 0; i < N; i++)
			for (size_t j = 0; j < M; j++)
				ret[j][i] = m_Data[i][j];

		return ret;
	}





	// non member operators
	template<class T, size_t N, size_t M>
	std::ostream& operator<<(std::ostream& stream, const Matrix<T, N, M>& matrix)
	{
		for (size_t i = 0; i < N; i++)
		{
			for (size_t j = 0; j < M; j++)
			{
				stream << matrix[i][j];

				if (j != M - 1)
					stream << " ";
			}

			stream << std::endl;
		}

		return stream;
	}




	template<class T, size_t N1, size_t M1, size_t N2, size_t M2, std::enable_if_t<M1 == N2, bool>>
	Matrix<T, N1, M2> operator*(const Matrix<T, N1, M1>& left, const Matrix<T, N2, M2>& right)
	{
		Matrix<T, N1, M2> ret;

		for (size_t i = 0; i < N1; i++)
			for (size_t j = 0; j < M2; j++)
				for (size_t k = 0; k < M1; k++)
					ret[i][j] += left[i][k] * right[k][j];

		return ret;
	}
}