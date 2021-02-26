#pragma once

#include <type_traits>
#include <cstddef>

#include "vector.h"
#include "leopphmath.h"



namespace leopph
{
	template<class T, size_t N, size_t M>
	class Matrix
	{
		static_assert(N >= 2 && M >= 2, "Matrix must be at least 2x2!");


	private:
		Vector<T, M> m_Data[N];


	public:
		// constructors
		Matrix();
		Matrix(const T& value);
		Matrix(const Matrix<T, N, M>& other);

		template<class... T1, std::enable_if_t<std::conjunction_v<std::is_convertible<T1, T>...> && sizeof...(T1) == (N > M ? M : N), bool> = false>
		Matrix(const T1&... args) :
			m_Data{}
		{
			T argArr[M > N ? N : M]{ static_cast<T>(args)... };

			for (size_t i = 0; i < M && i < N; i++)
				m_Data[i][i] = argArr[i];
		}

		template<class... T1, std::enable_if_t<std::conjunction_v<std::is_convertible<T1, T>...> && sizeof...(T1) == (N * M), bool> = false>
		Matrix(const T1&... args) :
			m_Data{}
		{
			T argArr[M * N]{ static_cast<T>(args)... };

			for (size_t i = 0; i < N; i++)
				for (size_t j = 0; j < M; j++)
					m_Data[i][j] = argArr[i * M + j];
		}

		template<std::size_t N1 = (M > N ? N : M), std::enable_if_t<N1 == (M > N ? N : M), bool> = false>
		Matrix(const Vector<T, N1>& vec) :
			m_Data{}
		{
			for (size_t i = 0; i < N1; i++)
				m_Data[i][i] = vec[i];
		}






		// factories
		template<size_t N1 = N, size_t M1 = M, std::enable_if_t<N1 == M1 && N1 == N && M1 == M, bool> = false>
		static Matrix<T, N, M> Identity()
		{
			return Matrix<T, N, M>{1};
		}






		// lookat matrix
		template<size_t N1 = N, size_t M1 = M, std::enable_if_t<N1 == M1 && N1 == N && M1 == M && N1 == 4, bool> = false>
		static Matrix<T, 4, 4> LookAt(const Vector<T, 3>& position, const Vector<T, 3>& target, const Vector<T, 3>& worldUp)
		{
			Vector<T, 3> z{ (target - position).Normalized() };
			Vector<T, 3> x{ Vector<T, 3>::Cross(worldUp, z).Normalized() };
			Vector<T, 3> y{ Vector<T, 3>::Cross(z, x) };

			return Matrix<T, 4, 4>
			{
				x[0],		y[0],		z[0],		0,
				x[1],		y[1],		z[1],		0,
				x[2],		y[2],		z[2],		0,
				-Vector3::Dot(x, position),		-Vector3::Dot(y, position),		-Vector3::Dot(z, position),		1
			};
		}






		// perspetive projection matrix
		template<size_t N1 = N, size_t M1 = M, std::enable_if_t<N1 == M1 && N1 == N && M1 == M && N1 == 4, bool> = false>
		static Matrix<T, 4, 4> Perspective(const T& fov, const T& aspectRatio, const T& nearClipPlane, const T& farClipPlane)
		{
			Matrix<T, 4, 4> ret{};

			T tanHalfFov{ static_cast<T>(Math::Tan(fov / static_cast<T>(2))) };

			ret[0][0] = static_cast<T>(1) / aspectRatio / tanHalfFov;
			ret[1][1] = static_cast<T>(1) / tanHalfFov;
			ret[2][2] = -(farClipPlane + nearClipPlane) / (farClipPlane - nearClipPlane);
			ret[2][3] = static_cast<T>(1);
			ret[3][2] = (2 * farClipPlane * nearClipPlane) / (farClipPlane - nearClipPlane);

			return ret;
		}






		// translation matrix
		template<size_t N1 = N, size_t M1 = M, std::enable_if_t<N1 == M1 && N1 == N && M1 == M && N1 == 4, bool> = false>
		static Matrix<T, 4, 4> Translate(const Vector<T, 3>& vector)
		{
			Matrix<T, 4, 4> ret = Identity();

			for (size_t i = 0; i < 3; i++)
				ret[3][i] = vector[i];

			return ret;
		}


		// scale matrix
		template<size_t N1 = N, size_t M1 = M, std::enable_if_t<N1 == M1 && N1 == N && M1 == M && N1 == 4, bool> = false>
		static Matrix<T, 4, 4> Scale(const Vector<T, 3>& vector)
		{
			Matrix<T, 4, 4> ret = Identity();

			for (size_t i = 0; i < 3; i++)
				ret[i][i] = vector[i];

			ret[3][3] = 1;

			return ret;
		}





		// get stored data as pointer
		const T* Data() const;
		T* Data();





		// member operators
		Matrix<T, N, M>& operator=(const Matrix<T, N, M>& other);
		const Vector<T, M>& operator[](size_t index) const;
		Vector<T, M>& operator[](size_t index);





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





		// TRANSPOSE
		Matrix<T, M, N> Transposed() const;

		template<size_t N1 = N, size_t M1 = M, std::enable_if_t<N1 == N && M1 == M && N1 == M1, bool> = false>
		Matrix<T, N, M> Transpose()
		{
			Matrix<T, N, M> transposed = Transposed();

			for (size_t i = 0; i < N; i++)
				for (size_t j = 0; j < M; j++)
					m_Data[i][j] = transposed[i][j];

			return *this;
		}




		// INVERSE
		template<size_t N1 = N, size_t M1 = M, std::enable_if_t<N1 == M1 && N1 == N && M1 == M, bool> = false>
		Matrix<T, N, M> Inverse() const
		{
			Matrix<T, N, M> copyOfThis{ *this };
			Matrix<T, N, M> inverse{ Matrix<T, N, M>::Identity() };

			for (std::size_t i = 0; i < N; i++)
			{
				if (copyOfThis[i][i] != static_cast<T>(1) && copyOfThis[i][i] != static_cast<T>(0))
				{
					T mult = copyOfThis[i][i];
					for (std::size_t k = 0; k < N; k++)
					{
						copyOfThis[i][k] /= mult;
						inverse[i][k] /= mult;
					}
				}
					
				for (std::size_t j = 0; j < N; j++)
					if (j != i)
					{
						T mult = copyOfThis[j][i] / copyOfThis[i][i];
						copyOfThis[j] -= mult * copyOfThis[i];
						inverse[j] -= mult * inverse[i];
					}
			}

			return inverse;
		}


		

		template<std::size_t N1 = N, std::size_t M1 = M, std::enable_if_t<N1 == M1 && N1 == N && M1 == M, bool> = false>
		operator Matrix<T, N - 1, N - 1>()
		{
			Matrix<T, N - 1, N - 1> ret{};

			for (std::size_t i = 0; i < N - 1; i++)
				for (std::size_t j = 0; j < N - 1; j++)
					ret[i][j] = m_Data[i][j];

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





	// aliases
	using Matrix2 = Matrix<float, 2, 2>;
	using Matrix3 = Matrix<float, 3, 3>;
	using Matrix4 = Matrix<float, 4, 4>;
}