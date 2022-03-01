#pragma once

#include "LeopphMath.hpp"
#include "Vector.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <limits>


namespace leopph
{
	namespace internal
	{
		// Template for matrices of different types and dimensions.
		// Provides seamsless interop with the Vector class template.
		// Matrices are represented as row-major and stored row-contigously.
		// N is the number of rows, M is the number of columns.
		// Should not be instantiated explicitly unless necessary.
		template<class T, std::size_t N, std::size_t M>
			requires(N > 1 && M > 1)
		class Matrix
		{
			public:
				constexpr Matrix() = default;

				// Fills the main diagonal with the passed value. Other elements are zero initialized.
				constexpr explicit Matrix(const T& value) noexcept;

				// Sets the main diagonal to the passed series of values. Other elements are zero initialized.
				template<std::convertible_to<T>... Args>
					requires(sizeof...(Args) == std::min(N, M))
				constexpr explicit(sizeof...(Args) <= 1) Matrix(const Args&... args) noexcept;

				// Sets the main diagonal to the elements of the passed Vector. Other elements are zero initialized.
				template<std::size_t K>
					requires(K == std::min(N, M))
				constexpr explicit Matrix(const Vector<T, K>& vec) noexcept;

				// Sets all elements row-contiguously to the passed series of values.
				template<std::convertible_to<T> ... Args>
					requires(sizeof...(Args) == N * M)
				constexpr explicit(sizeof...(Args) <= 1) Matrix(const Args&... args) noexcept;

				// Construct a Matrix that has one less row and column than the passed Matrix by dropping its last row and column.
				constexpr explicit Matrix(const Matrix<T, N + 1, M + 1>& other);

				constexpr Matrix(const Matrix<T, N, M>& other) = default;
				constexpr auto operator=(const Matrix& other) -> Matrix<T, N, M>& = default;

				constexpr Matrix(Matrix<T, N, M>&& other) = default;
				constexpr auto operator=(Matrix&& other) -> Matrix<T, N, M>& = default;

				constexpr ~Matrix() = default;

				// Indetity matrix.
				constexpr static auto Identity() noexcept -> Matrix<T, N, M>
					requires(N == M);

				// View matrix for rendering that is calculated based on current Position, target Position, and the world's vertical axis.
				static auto LookAt(const Vector<T, 3>& position, const Vector<T, 3>& target, const Vector<T, 3>& worldUp) noexcept -> Matrix<T, 4, 4>
					requires(N == 4 && M == 4);

				// Perspective proection matrix for rendering that is calculated based on the left, right, top, and bottom coordinates of the view frustum as well as near and far clip planes.
				constexpr static auto Perspective(const T& left, const T& right, const T& top, const T& bottom, const T& nearClipPlane, const T& farClipPlane) noexcept -> Matrix<T, 4, 4>
					requires (N == 4 && M == 4);

				// Perspective projection matrix for rendering that is calculated based on FOV, aspect ratio, and the near and far clip planes.
				static auto Perspective(const T& fov, const T& aspectRatio, const T& nearClipPlane, const T& farClipPlane) noexcept -> Matrix<T, 4, 4>
					requires (N == 4 && M == 4);

				// Orthographgic projection matrix for rendering that is calculated based on the left, right, top, and bottom coordinates of the view frustum.
				constexpr static auto Ortographic(const T& left, const T& right, const T& top, const T& bottom, const T& nearClipPlane, const T& farClipPlane) noexcept -> Matrix<T, 4, 4>;

				// Translation matrix.
				constexpr static auto Translate(const Vector<T, 3>& vector) noexcept -> Matrix<T, 4, 4>
					requires (N == 4 && M == 4);

				// Scaling matrix.
				constexpr static auto Scale(const Vector<T, 3>& vector) noexcept -> Matrix<T, 4, 4>
					requires (N == 4 && M == 4);

				// Returns a reference to the internal data structure.
				// It is not adviced to call this function.
				[[nodiscard]] constexpr auto Data() const noexcept -> auto&;

				// Returns the Nth row of the Matrix as a const reference to an M dimensional Vector.
				constexpr auto operator[](size_t index) const -> auto&;
				// Returns the Nth row of the Matrix as a non-const reference to an M dimensional Vector.
				constexpr auto operator[](size_t index) -> auto&;

				// Determinant of the Matrix.
				[[nodiscard]] constexpr auto Det() const noexcept -> T
					requires(N == M);

				// Returns a new Matrix that is the transposed of this Matrix.
				[[nodiscard]] constexpr auto Transposed() const noexcept -> Matrix<T, M, N>;

				// In-place transpose.
				// Returns a reference to this Matrix.
				constexpr auto Transpose() noexcept -> Matrix<T, N, M>&
					requires(N == M);

				// Returns a new Matrix that is the inverse of this Matrix.
				// The function does not check whether the inverse exists or not. Make sure to only call this if it does.
				[[nodiscard]] constexpr auto Inverse() const noexcept -> Matrix<T, N, M>
					requires(N == M);

				// Returns a new Matrix that has one more rows and colums than this.
				// The new elements are all zero initialized, except the one in the main diagonal, which will be 1.
				constexpr explicit operator Matrix<T, N + 1, N + 1>() const noexcept;

			private:
				// Helper function to get const and non-const references to elements depending on context.
				[[nodiscard]] constexpr static auto GetElementCommon(auto* self, std::size_t index) -> decltype(auto);
				// For integers this is equal to elem == 0.
				// For others it is equal to abs(elem) < epsilon.
				[[nodiscard]] constexpr static auto IsZero(const T& elem) -> bool;

				std::array<Vector<T, M>, N> m_Data{};
		};


		// Definitions

		template<class T, std::size_t N, std::size_t M>
			requires (N > 1 && M > 1)
		template<std::size_t K>
			requires (K == std::min(N, M))
		constexpr Matrix<T, N, M>::Matrix(const Vector<T, K>& vec) noexcept
		{
			for (size_t i = 0; i < K; i++)
			{
				m_Data[i][i] = vec[i];
			}
		}


		template<class T, std::size_t N, std::size_t M>
			requires (N > 1 && M > 1)
		template<std::convertible_to<T> ... Args>
			requires (sizeof...(Args) == N * M)
		constexpr Matrix<T, N, M>::Matrix(const Args&... args) noexcept
		{
			T argArr[M * N]{static_cast<T>(args)...};
			for (size_t i = 0; i < N; i++)
			{
				for (size_t j = 0; j < M; j++)
				{
					m_Data[i][j] = argArr[i * M + j];
				}
			}
		}


		template<class T, std::size_t N, std::size_t M>
			requires (N > 1 && M > 1)
		constexpr Matrix<T, N, M>::Matrix(const Matrix<T, N + 1, M + 1>& other)
		{
			for (std::size_t i = 0; i < N; ++i)
			{
				m_Data[i] = static_cast<Vector<T, M>>(other[i]);
			}
		}


		template<class T, std::size_t N, std::size_t M>
			requires (N > 1 && M > 1)
		constexpr auto Matrix<T, N, M>::Identity() noexcept -> Matrix<T, N, M>
			requires (N == M)
		{
			return Matrix<T, N, M>{1};
		}


		template<class T, std::size_t N, std::size_t M>
			requires (N > 1 && M > 1)
		auto Matrix<T, N, M>::LookAt(const Vector<T, 3>& position, const Vector<T, 3>& target, const Vector<T, 3>& worldUp) noexcept -> Matrix<T, 4, 4>
			requires (N == 4 && M == 4)
		{
			Vector<T, 3> z{(target - position).Normalized()};
			Vector<T, 3> x{Vector<T, 3>::Cross(worldUp, z).Normalized()};
			Vector<T, 3> y{Vector<T, 3>::Cross(z, x)};
			return Matrix<T, 4, 4>
			{
				x[0], y[0], z[0], 0,
				x[1], y[1], z[1], 0,
				x[2], y[2], z[2], 0,
				-Vector3::Dot(x, position), -Vector3::Dot(y, position), -Vector3::Dot(z, position), 1
			};
		}


		template<class T, std::size_t N, std::size_t M>
			requires (N > 1 && M > 1)
		constexpr auto Matrix<T, N, M>::Perspective(const T& left, const T& right, const T& top, const T& bottom, const T& nearClipPlane, const T& farClipPlane) noexcept -> Matrix<T, 4, 4>
			requires (N == 4 && M == 4)
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


		template<class T, std::size_t N, std::size_t M>
			requires (N > 1 && M > 1)
		auto Matrix<T, N, M>::Perspective(const T& fov, const T& aspectRatio, const T& nearClipPlane, const T& farClipPlane) noexcept -> Matrix<T, 4, 4>
			requires (N == 4 && M == 4)
		{
			T tanHalfFov{static_cast<T>(math::Tan(fov / static_cast<T>(2)))};
			T top{nearClipPlane * tanHalfFov};
			T bottom{-top};
			T right{top * aspectRatio};
			T left{-right};
			return Perspective(left, right, top, bottom, nearClipPlane, farClipPlane);
		}


		template<class T, std::size_t N, std::size_t M>
			requires (N > 1 && M > 1)
		constexpr auto Matrix<T, N, M>::Ortographic(const T& left, const T& right, const T& top, const T& bottom, const T& nearClipPlane, const T& farClipPlane) noexcept -> Matrix<T, 4, 4>
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


		template<class T, std::size_t N, std::size_t M>
			requires (N > 1 && M > 1)
		constexpr auto Matrix<T, N, M>::Translate(const Vector<T, 3>& vector) noexcept -> Matrix<T, 4, 4>
			requires (N == 4 && M == 4)
		{
			Matrix<T, 4, 4> ret = Identity();
			for (size_t i = 0; i < 3; i++)
			{
				ret[3][i] = vector[i];
			}
			return ret;
		}


		template<class T, std::size_t N, std::size_t M>
			requires (N > 1 && M > 1)
		constexpr auto Matrix<T, N, M>::Scale(const Vector<T, 3>& vector) noexcept -> Matrix<T, 4, 4>
			requires (N == 4 && M == 4)
		{
			Matrix<T, 4, 4> ret = Identity();
			for (size_t i = 0; i < 3; i++)
			{
				ret[i][i] = vector[i];
			}
			return ret;
		}


		template<class T, std::size_t N, std::size_t M>
			requires (N > 1 && M > 1)
		constexpr auto Matrix<T, N, M>::Data() const noexcept -> auto&
		{
			return m_Data;
		}


		template<class T, std::size_t N, std::size_t M>
			requires (N > 1 && M > 1)
		constexpr auto Matrix<T, N, M>::operator[](size_t index) const -> auto&
		{
			return GetElementCommon(this, index);
		}


		template<class T, std::size_t N, std::size_t M>
			requires (N > 1 && M > 1)
		constexpr auto Matrix<T, N, M>::operator[](const size_t index) -> auto&
		{
			return GetElementCommon(this, index);
		}


		template<class T, std::size_t N, std::size_t M>
			requires (N > 1 && M > 1)
		constexpr auto Matrix<T, N, M>::Det() const noexcept -> T
			requires (N == M)
		{
			Matrix<T, N, M> tmp{*this};
			for (size_t i = 1; i < N; i++)
			{
				for (size_t j = 0; j < N; j++)
				{
					tmp[j][0] += tmp[j][i];
				}
			}
			for (size_t i = 1; i < N; i++)
			{
				tmp[i] -= tmp[0];
			}
			auto ret{static_cast<T>(1)};
			for (size_t i = 0; i < N; i++)
			{
				ret *= tmp[i][i];
			}
			return ret;
		}


		template<class T, std::size_t N, std::size_t M>
			requires (N > 1 && M > 1)
		constexpr auto Matrix<T, N, M>::Transposed() const noexcept -> Matrix<T, M, N>
		{
			Matrix<T, M, N> ret;
			for (size_t i = 0; i < N; i++)
			{
				for (size_t j = 0; j < M; j++)
				{
					ret[j][i] = m_Data[i][j];
				}
			}
			return ret;
		}


		template<class T, std::size_t N, std::size_t M>
			requires (N > 1 && M > 1)
		constexpr auto Matrix<T, N, M>::Transpose() noexcept -> Matrix<T, N, M>&
			requires (N == M)
		{
			for (std::size_t i = 0; i < N; i++)
			{
				for (auto j = i + 1; j < M; j++)
				{
					auto temp{m_Data[i][j]};
					m_Data[i][j] = m_Data[j][i];
					m_Data[j][i] = temp;
				}
			}
			return *this;
		}


		template<class T, std::size_t N, std::size_t M>
			requires (N > 1 && M > 1)
		constexpr auto Matrix<T, N, M>::Inverse() const noexcept -> Matrix<T, N, M>
			requires (N == M)
		{
			Matrix<T, N, M> left{*this};
			Matrix<T, N, M> right{Matrix<T, N, M>::Identity()};

			// Iterate over the main diagonal/submatrices
			for (std::size_t i = 0; i < N; i++)
			{
				// Try to correct zero element in main diagonal
				if (IsZero(left[i][i]))
				{
					// Find a non-zero element below the tested one and swap rows with it
					for (auto j = i + 1; j < N; j++)
					{
						if (!IsZero(left[j][i]))
						{
							// Swap rows in left
							auto tmp{left[j]};
							left[j] = left[i];
							left[i] = tmp;

							// Swap rows in right
							tmp = right[j];
							right[j] = right[i];
							right[i] = tmp;
							break;
						}
					}
				}

				// If the main diagonal element is non-zero
				// 1. Normalize the row so that the element is 1
				// 2. Reduce all rows below so that its elements under are zero
				if (!IsZero(left[i][i]))
				{
					const auto div{left[i][i]};
					left[i] /= div;
					right[i] /= div;

					for (auto j = i + 1; j < N; j++)
					{
						const auto mult{left[j][i] / left[i][i]}; // Theoretically left[i][i] is 1 by now but for the sake of float accuracy we do the division
						left[j] -= mult * left[i];
						right[j] -= mult * right[i];
					}
				}
			}

			// Left is in reduced echelon form
			// Now eliminate the remaining non-zeros outside the main diagonal
			for (std::size_t i = 0; i < N; i++)
			{
				for (auto j = i + 1; j < N; j++)
				{
					if (!IsZero(left[i][j]))
					{
						// We subtract the jth row from the ith one
						// Because it is guaranteed to have 0s before the main diagonal
						// And thus it won't mess up the element in the ith row before the jth element
						const auto mult{left[i][j] / left[j][j]}; // left[j][j] is theoretically 1 but for the sake of float accuracy we do the division
						left[i] -= mult * left[j];
						right[i] -= mult * right[j];
					}
				}
			}
			return right;
		}


		template<class T, std::size_t N, std::size_t M>
			requires (N > 1 && M > 1)
		constexpr Matrix<T, N, M>::operator Matrix<T, N + 1, N + 1>() const noexcept
		{
			Matrix<T, N + 1, N + 1> ret;
			for (std::size_t i = 0; i < N; i++)
			{
				for (std::size_t j = 0; j < N; j++)
				{
					ret[i][j] = m_Data[i][j];
				}
			}
			ret[N][N] = static_cast<T>(1);
			return ret;
		}


		template<class T, std::size_t N, std::size_t M>
			requires (N > 1 && M > 1)
		constexpr Matrix<T, N, M>::Matrix(const T& value) noexcept
		{
			for (size_t i = 0; i < N && i < M; i++)
			{
				m_Data[i][i] = value;
			}
		}


		template<class T, std::size_t N, std::size_t M>
			requires (N > 1 && M > 1)
		template<std::convertible_to<T> ... Args>
			requires (sizeof...(Args) == std::min(N, M))
		constexpr Matrix<T, N, M>::Matrix(const Args&... args) noexcept
		{
			T argArr[M > N ? N : M]{static_cast<T>(args)...};
			for (size_t i = 0; i < M && i < N; i++)
			{
				m_Data[i][i] = argArr[i];
			}
		}


		template<class T, std::size_t N, std::size_t M>
			requires (N > 1 && M > 1)
		constexpr auto Matrix<T, N, M>::GetElementCommon(auto* const self, const std::size_t index) -> decltype(auto)
		{
			return self->m_Data[index];
		}


		template<class T, std::size_t N, std::size_t M>
			requires (N > 1 && M > 1)
		constexpr auto Matrix<T, N, M>::IsZero(const T& elem) -> bool
		{
			if constexpr (std::numeric_limits<T>::is_integer)
			{
				return elem == static_cast<T>(0);
			}
			else
			{
				return std::abs(elem) < std::numeric_limits<T>::epsilon();
			}
		}


		// Non-member operators

		template<class T, std::size_t N, std::size_t M>
		constexpr auto operator<<(std::ostream& stream, const Matrix<T, N, M>& matrix) -> std::ostream&
		{
			for (size_t i = 0; i < N; i++)
			{
				for (size_t j = 0; j < M; j++)
				{
					stream << matrix[i][j];
					if (j != M - 1)
					{
						stream << " ";
					}
				}
				if (i != N - 1)
				{
					stream << std::endl;
				}
			}
			return stream;
		}


		template<class T, std::size_t N, std::size_t M>
		constexpr auto operator+(const Matrix<T, N, M>& left, const Matrix<T, N, M>& right) noexcept -> Matrix<T, N, M>
		{
			Matrix<T, N, M> ret;
			for (std::size_t i = 0; i < N; i++)
			{
				ret[i] = left[i] + right[i];
			}
			return ret;
		}


		template<class T, std::size_t N, std::size_t M>
		constexpr auto operator+=(Matrix<T, N, M>& left, const Matrix<T, N, M>& right) noexcept -> Matrix<T, N, M>&
		{
			for (std::size_t i = 0; i < N; i++)
			{
				left[i] += right[i];
			}
			return left;
		}


		template<class T, std::size_t N, std::size_t M>
		constexpr auto operator-(const Matrix<T, N, M>& left, const Matrix<T, N, M>& right) noexcept -> Matrix<T, N, M>
		{
			Matrix<T, N, M> ret;
			for (std::size_t i = 0; i < N; i++)
			{
				ret[i] = left[i] - right[i];
			}
			return ret;
		}


		template<class T, std::size_t N, std::size_t M>
		constexpr auto operator-=(Matrix<T, N, M>& left, const Matrix<T, N, M>& right) noexcept -> Matrix<T, N, M>&
		{
			for (std::size_t i = 0; i < N; i++)
			{
				left[i] -= right[i];
			}
			return left;
		}


		template<class T, std::size_t N, std::size_t M, std::convertible_to<T> T1>
		constexpr auto operator*(const Matrix<T, N, M>& left, const T1& right) noexcept -> Matrix<T, N, M>
		{
			Matrix<T, N, M> ret;
			for (std::size_t i = 0; i < N; i++)
			{
				ret[i] = left[i] * static_cast<T>(right);
			}
			return ret;
		}


		template<class T, std::size_t N, std::size_t M, std::convertible_to<T> T1>
		constexpr auto operator*(const T1& left, const Matrix<T, N, M>& right) noexcept -> Matrix<T, N, M>
		{
			Matrix<T, N, M> ret;
			for (std::size_t i = 0; i < N; i++)
			{
				ret[i] = static_cast<T>(left) * right[i];
			}
			return ret;
		}


		template<class T, std::size_t N, std::size_t M>
		constexpr auto operator*(const Matrix<T, N, M>& left, const Vector<T, M>& right) noexcept -> Vector<T, N>
		{
			Vector<T, N> ret;
			for (size_t i = 0; i < N; i++)
			{
				for (size_t j = 0; j < M; j++)
				{
					ret[i] += left[i][j] * right[j];
				}
			}
			return ret;
		}


		template<class T, std::size_t N, std::size_t M>
		constexpr auto operator*(const Vector<T, N>& left, const Matrix<T, N, M>& right) noexcept -> Vector<T, M>
		{
			Vector<T, M> ret;
			for (size_t j = 0; j < M; j++)
			{
				for (size_t i = 0; i < N; i++)
				{
					ret[j] += left[i] * right[i][j];
				}
			}
			return ret;
		}


		template<class T, std::size_t N, std::size_t M, std::size_t P>
		constexpr auto operator*(const Matrix<T, N, M>& left, const Matrix<T, M, P>& right) noexcept -> Matrix<T, N, P>
		{
			Matrix<T, N, P> ret;
			for (size_t i = 0; i < N; i++)
			{
				for (size_t k = 0; k < P; k++)
				{
					for (size_t j = 0; j < M; j++)
					{
						ret[i][k] += left[i][j] * right[j][k];
					}
				}
			}
			return ret;
		}


		template<class T, std::size_t N, std::size_t M, std::convertible_to<T> T1>
		constexpr auto operator*=(Matrix<T, N, M>& left, const T1& right) noexcept -> Matrix<T, N, M>&
		{
			for (std::size_t i = 0; i < N; i++)
			{
				left[i] *= static_cast<T>(right);
			}
			return left;
		}


		template<class T, std::size_t N>
		constexpr auto operator*=(Vector<T, N>& left, const Matrix<T, N, N>& right) noexcept -> Vector<T, N>&
		{
			return left = left * right;
		}


		template<class T, std::size_t N, std::size_t M>
		constexpr auto operator*=(Matrix<T, N, M>& left, const Matrix<T, M, M>& right) noexcept -> Matrix<T, N, M>&
		{
			return left = left * right;
		}
	}


	// 2x2 Matrix of floats
	using Matrix2 = internal::Matrix<float, 2, 2>;
	// 3x3 Matrix of floats
	using Matrix3 = internal::Matrix<float, 3, 3>;
	// 4x4 Matrix of floats
	using Matrix4 = internal::Matrix<float, 4, 4>;
}
