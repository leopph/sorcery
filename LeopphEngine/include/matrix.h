#pragma once

#include <type_traits>
#include <memory>

#include "vector.h"
#include "leopphmath.h"



namespace leopph::implementation
{
	template<class T, size_t N, size_t M>
	class Matrix
	{
		static_assert(N >= 2 && M >= 2, "Matrix must be at least 2x2!");


	private:
		Vector<T, M> m_Data[N];


	public:
		// constructors
		Matrix() :
			m_Data{}
		{}

		Matrix(const T& value) :
			m_Data{}
		{
			for (size_t i = 0; i < N && i < M; i++)
				m_Data[i][i] = value;
		}

		template<class... T1, std::enable_if_t<std::conjunction_v<std::is_convertible<T1, T>...> && sizeof...(T1) == (N > M ? M : N), bool> = false>
		Matrix(const T1&... args) :
			m_Data{}
		{
			// TODO think this over man

			T argArr[M > N ? N : M]{ static_cast<T>(args)... };

			for (size_t i = 0; i < M && i < N; i++)
				m_Data[i][i] = argArr[i];
		}

		template<class... T1, std::enable_if_t<std::conjunction_v<std::is_convertible<T1, T>...> && sizeof...(T1) == (N * M), bool> = false>
		Matrix(const T1&... args) :
			m_Data{}
		{
			// TODO rework candidate

			T argArr[M * N]{ static_cast<T>(args)... };

			for (size_t i = 0; i < N; i++)
				for (size_t j = 0; j < M; j++)
					m_Data[i][j] = argArr[i * M + j];
		}

		Matrix(const Matrix<T, N, M>& other) :
			m_Data{}
		{
			for (size_t i = 0; i < N; i++)
				m_Data[i] = other.m_Data[i];
		}


		// factories
		template<size_t N1 = N, size_t M1 = M, std::enable_if_t<N1 == M1 && N1 == N && M1 == M, bool> = false>
		static Matrix<T, N, M> Identity()
		{
			return Matrix<T, N, M>{1};
		}


		// lookat matrix
		template<size_t N1 = N, size_t M1 = M, std::enable_if_t<N1 == M1 && N1 == N && M1 == M && N1 == 4, bool> = false>
		static Matrix<T, 4, 4> LookAt(const Vector<T, 3>& position, const Vector<T, 3>& forward, const Vector<T, 3>& worldUp)
		{
			Vector<T, 3> z{ forward.Normalized() };
			Vector<T, 3> x{ Vector<T, 3>::Cross(worldUp, z) };
			Vector<T, 3> y{ Vector<T, 3>::Cross(z, x) };

			return Matrix<T, 4, 4>
			{
				x[0], x[1], x[2], -Vector<T, 3>::Dot(x, position),
					y[0], y[1], y[2], -Vector<T, 3>::Dot(y, position),
					z[0], z[1], z[2], -Vector<T, 3>::Dot(z, position),
					0, 0, 0, 1
			};
		}



		// perspetive projection matrix
		template<size_t N1 = N, size_t M1 = M, std::enable_if_t<N1 == M1 && N1 == N && M1 == M && N1 == 4, bool> = false>
		static Matrix<T, 4, 4> Perspective(const T& fov, const T& aspectRatio, const T& nearClipPlane, const T& farClipPlane)
		{
			Matrix<T, 4, 4> ret{};

			T tanHalfFov{ static_cast<T>(Math::Tan(fov / static_cast<T>(2))) };

			ret[0][0] = static_cast<T>(1) / (aspectRatio * tanHalfFov);
			ret[1][1] = static_cast<T>(1) / tanHalfFov;
			ret[2][2] = (nearClipPlane + farClipPlane) / (farClipPlane - nearClipPlane);
			ret[3][2] = static_cast<T>(1);
			ret[2][3] = (static_cast<T>(2) * farClipPlane * nearClipPlane) / (farClipPlane - nearClipPlane);

			return ret;
		}





		// get stored data as pointer
		std::unique_ptr<T[]> Data() const
		{
			std::unique_ptr<T[]> ret{ new T[N * M] };

			for (size_t i = 0; i < N; i++)
				for (size_t j = 0; j < M; j++)
					ret[i * j + j] = m_Data[i][j];

			return ret;
		}





		// member operators
		Matrix<T, N, M>& operator=(const Matrix<T, N, M>& other)
		{
			if (this == &other)
				return *this;

			for (size_t i = 0; i < N; i++)
				for (size_t j = 0; j < M; j++)
					m_Data[i][j] = other.m_Data[i][j];

			return *this;
		}

		const Vector<T, M>& operator[](size_t index) const
		{
			return m_Data[index];
		}

		Vector<T, M>& operator[](size_t index)
		{
			return const_cast<Vector<T, M>&>(const_cast<const Matrix<T, N, M>*>(this)->operator[](index));
		}


		// determinant
		template<size_t N1 = N, size_t M1 = M, std::enable_if_t<N1 == M1 && N1 == N && M1 == M, bool> = false>
		float Det() const
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
		Matrix<T, M, N> Transposed() const
		{
			Matrix<T, M, N> ret;

			for (size_t i = 0; i < N; i++)
				for (size_t j = 0; j < M; j++)
					ret[j][i] = m_Data[i][j];

			return ret;
		}
	};





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

			if (i != N - 1)
				stream << std::endl;
		}

		return stream;
	}



	template<class T, size_t N1, size_t M1, size_t N2, size_t M2, std::enable_if_t<M1 == N2, bool> = false>
	Matrix<T, N1, M2> operator*(const Matrix<T, N1, M1>& left, const Matrix<T, N2, M2>& right)
	{
		Matrix<T, N1, M2> ret;

		for (size_t i = 0; i < N1; i++)
			for (size_t j = 0; j < M2; j++)
				for (size_t k = 0; k < M1; k++)
					ret[i][j] += left[i][k] * right[k][j];

		return ret;
	}



	template<class T, size_t N1, size_t M1, size_t N2, size_t M2, std::enable_if_t<M1 == N2 && N1 == M1 && N2 == M2, bool> = false>
	Matrix<T, N1, M1>& operator*=(Matrix<T, N1, M1>& left, const Matrix<T, N2, M2>& right)
	{
		return left = left * right;
	}



	template<class T, size_t P, size_t N, size_t M, std::enable_if_t<P == N, bool> = false>
	Vector<T, M> operator*(const Vector<T, P>& left, const Matrix<T, N, M>& right)
	{
		Vector<T, M> ret;

		for (size_t j = 0; j < M; j++)
			for (size_t k = 0; k < P; k++)
				ret[j] += left[k] * right[k][j];

		return ret;
	}


	template<class T, size_t N, size_t M, size_t P, std::enable_if_t<M == P, bool> = false>
	Vector<T, N> operator*(const Matrix<T, N, M>& left, const Vector<T, P>& right)
	{
		Vector<T, N> ret;

		for (size_t i = 0; i < N; i++)
			for (size_t k = 0; k < M; k++)
				ret[i] += left[i][k] * right[k];

		return ret;
	}





	// instantiations
	template class Matrix<float, 2, 2>;
	template class Matrix<float, 3, 3>;
	template class Matrix<float, 4, 4>;
}

namespace leopph
{
	// aliases
	using Matrix2 = implementation::Matrix<float, 2, 2>;
	using Matrix3 = implementation::Matrix<float, 3, 3>;
	using Matrix4 = implementation::Matrix<float, 4, 4>;
}