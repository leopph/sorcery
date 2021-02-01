#pragma once

#include <type_traits>

#include "vector.h"
#include "leopphapi.h"



namespace leopph
{
	template<class T, size_t N, size_t M>
	class Matrix
	{
		static_assert(N >= 2 && M >= 2, "Matrix must be at least 2x2!");


	private:
		Vector<T, M>* m_Data;


	public:
		// constructors
		Matrix();

		Matrix(const T& value);

		template<class... T1, std::enable_if_t<std::conjunction_v<std::is_convertible<T1, T>...> && sizeof...(T1) == (N > M ? M : N), bool> = false>
		Matrix(const T1&... args);

		template<class... T1, std::enable_if_t<std::conjunction_v<std::is_convertible<T1, T>...> && sizeof...(T1) == (N * M), bool> = false>
		Matrix(const T1&... args);


		// factories
		template<size_t N1 = N, size_t M1 = M, std::enable_if_t<N1 == M1 && N1 == N && M1 == M, bool> = false>
		static Matrix<T, N, M> Identity();


		// destructor
		~Matrix();


		// member operators
		Matrix<T, N, M>& operator=(const Matrix<T, N, M>& other);

		const Vector<T, M>& operator[](size_t index) const;
		Vector<T, M>& operator[](size_t index);


		// determinant
		template<size_t N1 = N, size_t M1 = M, std::enable_if_t<N1 == M1 && N1 == N && M1 == M, bool> = false>
		float Det() const;


		// transposed copy
		Matrix<T, M, N> Transposed() const;
	};





	// non member operators
	template<class T, size_t N, size_t M>
	std::ostream& operator<<(std::ostream& stream, const Matrix<T, N, M>& matrix);



	template<class T, size_t N1, size_t M1, size_t N2, size_t M2, std::enable_if_t<M1 == N2, bool> = false>
	Matrix<T, N1, M2> operator*(const Matrix<T, N1, M1>& left, const Matrix<T, N2, M2>& right);





	// instantiations
	template class LEOPPHAPI Matrix<float, 2, 2>;
	template class LEOPPHAPI Matrix<float, 3, 3>;
	template class LEOPPHAPI Matrix<float, 4, 4>;


	// aliases
	using Matrix2 = Matrix<float, 2, 2>;
	using Matrix3 = Matrix<float, 3, 3>;
	using Matrix4 = Matrix<float, 4, 4>;
}