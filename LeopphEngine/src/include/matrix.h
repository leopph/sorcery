#pragma once

#include <cstddef>
#include <concepts>

#include "vector.h"
#include "leopphmath.h"



namespace leopph
{
	namespace implementation
	{
		template<class T, std::size_t N, std::size_t M> requires(N > 1 && M > 1)
		class Matrix
		{
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

			Matrix(const Matrix<T, N, M>& other) :
				m_Data{}
			{
				for (size_t i = 0; i < N; i++)
					m_Data[i] = other.m_Data[i];
			}

			template<std::convertible_to<T>... T1> requires(sizeof...(T1) == (N > M ? M : N))
				Matrix(const T1&... args) :
				m_Data{}
			{
				T argArr[M > N ? N : M]{ static_cast<T>(args)... };

				for (size_t i = 0; i < M && i < N; i++)
					m_Data[i][i] = argArr[i];
			}

			template<std::convertible_to<T> ... T1> requires(sizeof...(T1) == (N * M))
				Matrix(const T1&... args) :
				m_Data{}
			{
				T argArr[M * N]{ static_cast<T>(args)... };

				for (size_t i = 0; i < N; i++)
					for (size_t j = 0; j < M; j++)
						m_Data[i][j] = argArr[i * M + j];
			}

			template<std::size_t N1> requires(N1 == (M > N ? N : M))
				Matrix(const Vector<T, N1>& vec) :
				m_Data{}
			{
				for (size_t i = 0; i < N1; i++)
					m_Data[i][i] = vec[i];
			}






			// factories
			static Matrix<T, N, M> Identity() requires(N == M)
			{
				return Matrix<T, N, M>{1};
			}






			// lookat matrix
			static Matrix<T, 4, 4> LookAt(const Vector<T, 3>& position, const Vector<T, 3>& target, const Vector<T, 3>& worldUp) requires(N == 4 && M == 4)
			{
				Vector<T, 3> z{ (target - position).Normalized() };
				Vector<T, 3> x{ Vector<T, 3>::Cross(worldUp, z).Normalized() };
				Vector<T, 3> y{ Vector<T, 3>::Cross(z, x) };

				return Matrix<T, 4, 4>
				{
					x[0], y[0], z[0], 0,
						x[1], y[1], z[1], 0,
						x[2], y[2], z[2], 0,
						-Vector3::Dot(x, position), -Vector3::Dot(y, position), -Vector3::Dot(z, position), 1
				};
			}






			// PERSPECTIVE PROJECTION
			static Matrix<T, 4, 4> Perspective(const T& left, const T& right, const T& top, const T& bottom, const T& nearClipPlane, const T& farClipPlane) requires (N == 4 && M == 4)
			{
				Matrix<T, 4, 4> ret;

				ret[0][0] = (static_cast<T>(2) * nearClipPlane) / (right - left);
				ret[2][0] = (right + left) / (right - left);
				ret[1][1] = (static_cast<T>(2) * nearClipPlane) / (top - bottom);
				ret[2][1] = (top + bottom) / (top - bottom);
				ret[2][2] = (farClipPlane + nearClipPlane) / (farClipPlane - nearClipPlane);
				ret[2][3] = static_cast<T>(1);
				ret[3][2] = (static_cast<T>(-2) * farClipPlane * nearClipPlane) / (farClipPlane - nearClipPlane);

				return ret;

			}


			static Matrix<T, 4, 4> Perspective(const T& fov, const T& aspectRatio, const T& nearClipPlane, const T& farClipPlane)  requires (N == 4 && M == 4)
			{
				T tanHalfFov{ static_cast<T>(Math::Tan(fov / static_cast<T>(2))) };
				T top{ nearClipPlane * tanHalfFov };
				T bottom{ -top };
				T right{ top * aspectRatio };
				T left{ -right };

				return Perspective(left, right, top, bottom, nearClipPlane, farClipPlane);
			}









			// translation matrix
			static Matrix<T, 4, 4> Translate(const Vector<T, 3>& vector) requires (N == 4 && M == 4)
			{
				Matrix<T, 4, 4> ret = Identity();

				for (size_t i = 0; i < 3; i++)
					ret[3][i] = vector[i];

				return ret;
			}


			// scale matrix
			static Matrix<T, 4, 4> Scale(const Vector<T, 3>& vector) requires (N == 4 && M == 4)
			{
				Matrix<T, 4, 4> ret = Identity();

				for (size_t i = 0; i < 3; i++)
					ret[i][i] = vector[i];

				ret[3][3] = 1;

				return ret;
			}





			// get stored data as pointer
			const T* Data() const
			{
				return m_Data[0].Data();
			}
			T* Data()
			{
				return const_cast<T*>(const_cast<const Matrix<T, N, M>*>(this)->Data());
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
			float Det() const requires(N == M)
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
			Matrix<T, M, N> Transposed() const
			{
				Matrix<T, M, N> ret;

				for (size_t i = 0; i < N; i++)
					for (size_t j = 0; j < M; j++)
						ret[j][i] = m_Data[i][j];

				return ret;
			}

			Matrix<T, N, M> Transpose() requires(N == M)
			{
				Matrix<T, N, M> transposed = Transposed();

				for (size_t i = 0; i < N; i++)
					for (size_t j = 0; j < M; j++)
						m_Data[i][j] = transposed[i][j];

				return *this;
			}




			// INVERSE
			Matrix<T, N, M> Inverse() const requires(N == M)
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



			operator Matrix<T, N - 1, N - 1>() requires(N == M)
			{
				Matrix<T, N - 1, N - 1> ret{};

				for (std::size_t i = 0; i < N - 1; i++)
					for (std::size_t j = 0; j < N - 1; j++)
						ret[i][j] = m_Data[i][j];

				return ret;
			}
		};





		// non member operators
		template<class T, std::size_t N, std::size_t M>
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



		template<class T, std::size_t N, std::size_t M, std::size_t P>
		Matrix<T, N, P> operator*(const Matrix<T, N, M>& left, const Matrix<T, M, P>& right)
		{
			Matrix<T, N, P> ret;

			for (size_t i = 0; i < N; i++)
				for (size_t j = 0; j < P; j++)
					for (size_t k = 0; k < M; k++)
						ret[i][j] += left[i][k] * right[k][j];

			return ret;
		}



		template<class T, std::size_t N, std::size_t M>
		Matrix<T, N, M>& operator*=(Matrix<T, N, M>& left, const Matrix<T, M, M>& right)
		{
			return left = left * right;
		}



		template<class T, std::size_t N, std::size_t M>
		Vector<T, M> operator*(const Vector<T, N>& left, const Matrix<T, N, M>& right)
		{
			Vector<T, M> ret;

			for (size_t j = 0; j < M; j++)
				for (size_t k = 0; k < N; k++)
					ret[j] += left[k] * right[k][j];

			return ret;
		}


		template<class T, std::size_t N, std::size_t M>
		Vector<T, N> operator*(const Matrix<T, N, M>& left, const Vector<T, M>& right)
		{
			Vector<T, N> ret;

			for (size_t i = 0; i < N; i++)
				for (size_t k = 0; k < M; k++)
					ret[i] += left[i][k] * right[k];

			return ret;
		}
	}





	// aliases
	using Matrix2 = implementation::Matrix<float, 2, 2>;
	using Matrix3 = implementation::Matrix<float, 3, 3>;
	using Matrix4 = implementation::Matrix<float, 4, 4>;
}