#pragma once

#include <array>
#include <cstddef>
#include <concepts>

#include "Vector.hpp"
#include "LeopphMath.hpp"


namespace leopph
{
	namespace impl
	{
		/*-------------------------------------------------------------------------------------------------
		The Matrix class template provides several ways to aid in solving linear algebraic problems.
		It is built to work seamlessly with the Vector class template. See "Vector.hpp" for more information.
		Matrices are represented as row-major and stored row-continuously. They have N rows and M columns.
		DO NOT INSTANTIATE THIS TEMPLATE EXPLICITLY UNLESS NECESSARY!
		There are several predefined implementations at the bottom of this file.
		-------------------------------------------------------------------------------------------------*/
		template<class T, std::size_t N, std::size_t M> requires(N > 1 && M > 1)
		class Matrix
		{
			std::array<Vector<T, M>, N> m_Data;


		public:
			Matrix() :
				m_Data{}
			{}


			/* Main Diagonal Fill Constructor */
			explicit Matrix(const T& value) :
				m_Data{}
			{
				for (size_t i = 0; i < N && i < M; i++)
					m_Data[i][i] = value;
			}


			/* Main Diagonal Elements Constructor */
			template<std::convertible_to<T>... T1> requires(sizeof...(T1) == (N > M ? M : N))
				explicit Matrix(const T1&... args) :
				m_Data{}
			{
				T argArr[M > N ? N : M]{ static_cast<T>(args)... };

				for (size_t i = 0; i < M && i < N; i++)
					m_Data[i][i] = argArr[i];
			}


			/* All Elements Constructor */
			template<std::convertible_to<T> ... T1> requires(sizeof...(T1) == (N * M))
				explicit Matrix(const T1&... args) :
				m_Data{}
			{
				T argArr[M * N]{ static_cast<T>(args)... };

				for (size_t i = 0; i < N; i++)
					for (size_t j = 0; j < M; j++)
						m_Data[i][j] = argArr[i * M + j];
			}


			/* Main Diagonal Vector Constructor */
			template<std::size_t N1> requires(N1 == (M > N ? N : M))
				explicit Matrix(const Vector<T, N1>& vec) :
				m_Data{}
			{
				for (size_t i = 0; i < N1; i++)
					m_Data[i][i] = vec[i];
			}


			Matrix(const Matrix<T, N, M>& other) = default;
			Matrix(Matrix<T, N, M>&& other) = default;
			Matrix<T, N, M>& operator=(const Matrix& other) = default;
			Matrix<T, N, M>& operator=(Matrix&& other) = default;
			~Matrix() = default;


			/* Mathematical Identity Matrix */
			static Matrix<T, N, M> Identity() requires(N == M)
			{
				return Matrix<T, N, M>{1};
			}


			/* View Matrix for the rendering pipeline that is calculated based on current position, target position, and the world's vertical axis */
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


			/* Perspective Projection Matrix for the rendering pipeline that is calculated based on the left, right, top, and bottom coordinates of the view frustum
			as well as near and far clip planes */
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


			/* Perspective Projection Matrix for the rendering pipeline that is calculated based on FOV, aspect ratio, and the near and far clip planes */
			static Matrix<T, 4, 4> Perspective(const T& fov, const T& aspectRatio, const T& nearClipPlane, const T& farClipPlane)  requires (N == 4 && M == 4)
			{
				T tanHalfFov{ static_cast<T>(math::Tan(fov / static_cast<T>(2))) };
				T top{ nearClipPlane * tanHalfFov };
				T bottom{ -top };
				T right{ top * aspectRatio };
				T left{ -right };

				return Perspective(left, right, top, bottom, nearClipPlane, farClipPlane);
			}


			/* Orthographgic Projection Matrix for the rendering pipeline that is calculated based on
			 * the left, right, top, and bottom coordinates of the view frustum. */
			static Matrix<T, 4, 4> Ortographic(const T& left, const T& right, const T& top, const T& bottom, const T& nearClipPlane, const T& farClipPlane)
			{
				Matrix<T, 4, 4> ret;
				ret[0][0] = static_cast<T>(static_cast<T>(2) / (right - left));
				ret[1][1] = static_cast<T>(static_cast<T>(2) / (top - bottom));
				ret[2][2] = static_cast<T>(static_cast<T>(2) / (farClipPlane - nearClipPlane));
				ret[3][0] = static_cast<T>(-((right + left) / (right - left)));
				ret[3][1] = static_cast<T>(-((top + bottom) / (top - bottom)));
				ret[3][2] = static_cast<T>(-((farClipPlane + nearClipPlane) / (farClipPlane - nearClipPlane)));
				ret[3][3] = static_cast<T>(1);
				return ret;
			}


			/* Mathematical Translation Matrix */
			static Matrix<T, 4, 4> Translate(const Vector<T, 3>& vector) requires (N == 4 && M == 4)
			{
				Matrix<T, 4, 4> ret = Identity();

				for (size_t i = 0; i < 3; i++)
					ret[3][i] = vector[i];

				return ret;
			}


			/* Mathematical Scaling Matrix */
			static Matrix<T, 4, 4> Scale(const Vector<T, 3>& vector) requires (N == 4 && M == 4)
			{
				Matrix<T, 4, 4> ret = Identity();

				for (size_t i = 0; i < 3; i++)
					ret[i][i] = vector[i];

				return ret;
			}


			/* Returns a pointer to the internal data structure.
			DO NOT USE THIS UNLESS NECASSARY */
			[[nodiscard]] const T* Data() const
			{
				return m_Data[0].Data();
			}


			T* Data()
			{
				return const_cast<T*>(const_cast<const Matrix<T, N, M>*>(this)->Data());
			}


			/* Returns the Nth row of the Matrix as an M dimensional Vector */
			const Vector<T, M>& operator[](size_t index) const
			{
				return m_Data[index];
			}


			Vector<T, M>& operator[](const size_t index)
			{
				return const_cast<Vector<T, M>&>(const_cast<const Matrix<T, N, M>*>(this)->operator[](index));
			}


			/* Mathematical Matrix Determinant for square Matrices */
			[[nodiscard]] float Det() const requires(N == M)
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


			/* Returns a new Matrix that is the Mathematical Transposed of this Matrix */
			[[nodiscard]] Matrix<T, M, N> Transposed() const
			{
				Matrix<T, M, N> ret;

				for (size_t i = 0; i < N; i++)
					for (size_t j = 0; j < M; j++)
						ret[j][i] = m_Data[i][j];

				return ret;
			}


			/* Mathematical Transposition of the Matrix in-place. */
			Matrix<T, N, M> Transpose() requires(N == M)
			{
				Matrix<T, N, M> transposed = Transposed();

				for (size_t i = 0; i < N; i++)
					for (size_t j = 0; j < M; j++)
						m_Data[i][j] = transposed[i][j];

				return *this;
			}


			/* Returns a new Matrix that is the Mathematical Inverse of this Matrix */
			[[nodiscard]] Matrix<T, N, M> Inverse() const requires(N == M)
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


			/* Applicable to N*N Matrices (N > 2).
			Returns a new (N-1)*(N-1) Matrix that is created by dropping the
			Nth row and column of the original square Matrix */
			template<std::size_t N1 = N, std::size_t M1 = M> requires(N1 == M1 && N1 > 2)
			explicit operator Matrix<T, N1 - 1, N1 - 1>() const
			{
				Matrix<T, N - 1, N - 1> ret{};

				for (std::size_t i = 0; i < N - 1; i++)
					for (std::size_t j = 0; j < N - 1; j++)
						ret[i][j] = m_Data[i][j];

				return ret;
			}


			/* Applicable to N*N matrices.
			Returns a new (N+1)*(N+1) Matrix that is created by adding
			a new row and column to the Matrix. All new elements are zero,
			except the one in the main diagonal, which is 1. */
			explicit operator Matrix<T, N + 1, N + 1>() const requires (N == M)
			{
				Matrix<T, N + 1, N + 1> ret{};

				for (std::size_t i = 0; i < N; i++)
					for (std::size_t j = 0; j < N; j++)
						ret[i][j] = m_Data[i][j];

				ret[N][N] = static_cast<T>(1);
				return ret;
			}
		};


		/*-----------------------------------
		Other standard mathematical operators
		-----------------------------------*/
		
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


	/*------------------------------------------------------
	Use these instances where you can in your business logic
	to get the best compatibility and performance.
	------------------------------------------------------*/
	
	using Matrix2 = impl::Matrix<float, 2, 2>;
	using Matrix3 = impl::Matrix<float, 3, 3>;
	using Matrix4 = impl::Matrix<float, 4, 4>;
}