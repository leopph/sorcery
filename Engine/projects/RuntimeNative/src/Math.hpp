#pragma once

#include "Core.hpp"

#include <cmath>
#include <concepts>
#include <cstddef>
#include <limits>
#include <ostream>


namespace leopph
{
	LEOPPHAPI extern f32 const PI;

	[[nodiscard]] LEOPPHAPI f32 to_radians(f32 degrees);
	[[nodiscard]] LEOPPHAPI f32 to_degrees(f32 radians);

	[[nodiscard]] LEOPPHAPI bool is_power_of_two(u32 value);
	[[nodiscard]] LEOPPHAPI u32 next_power_of_two(u32 value);

	[[nodiscard]] LEOPPHAPI f32 lerp(f32 from, f32 to, f32 t);

	[[nodiscard]] LEOPPHAPI u8 num_binary_digits(u32 number);


	template<class T, std::size_t N> requires(N > 1)
		class Vector
	{
	public:
		[[nodiscard]] f32 length() const;
		[[nodiscard]] Vector<T, N> normalized() const;
		Vector<T, N>& normalize();


		[[nodiscard]] T const& operator[](size_t index) const;
		[[nodiscard]] T& operator[](size_t index);
		[[nodiscard]] T const* get_data() const;


		[[nodiscard]] static Vector<T, N> up();
		[[nodiscard]] static Vector<T, N> down();
		[[nodiscard]] static Vector<T, N> left();
		[[nodiscard]] static Vector<T, N> right();
		[[nodiscard]] static Vector<T, N> forward() requires(N >= 3);
		[[nodiscard]] static Vector<T, N> backward() requires(N >= 3);


		Vector() = default;

		template<std::convertible_to<T> T1>
		explicit Vector(T1 const& value);

		template<std::convertible_to<T>... Args> requires(sizeof...(Args) == N)
			explicit(sizeof...(Args) <= 1) Vector(Args const&... args);

		template<std::size_t M> requires (M > N)
			explicit Vector(Vector<T, M> const& other);

		template<std::size_t M> requires (M < N)
			explicit Vector(Vector<T, M> const& other, T const& fill);

		Vector(Vector<T, N> const& other) = default;
		Vector(Vector<T, N>&& other) noexcept = default;

		~Vector() = default;

		Vector<T, N>& operator=(Vector<T, N> const& other) = default;
		Vector<T, N>& operator=(Vector<T, N>&& other) noexcept = default;


	private:
		[[nodiscard]] static decltype(auto) get_element(auto* self, std::size_t index);

		T mData[N];
	};


	template<class T, std::size_t N> requires (N > 1)
		T dot(Vector<T, N> const& left, Vector<T, N> const& right);

	template<class T, std::size_t N> requires (N > 1)
		Vector<T, N> cross(Vector<T, N> const& left, Vector<T, N> const& right) requires(N == 3);

	template<class T, std::size_t N> requires (N > 1)
		f32 distance(Vector<T, N> const& left, Vector<T, N> const& right);

	template<class T, std::size_t N>
	Vector<T, N> lerp(Vector<T, N> const& from, Vector<T, N> const& to, float t);

	template<class T, std::size_t N>
	Vector<T, N> operator-(Vector<T, N> const& operand);

	template<class T, std::size_t N>
	Vector<T, N> operator+(Vector<T, N> const& left, Vector<T, N> const& right);

	template<class T, std::size_t N>
	Vector<T, N>& operator+=(Vector<T, N>& left, Vector<T, N> const& right);

	template<class T, std::size_t N>
	Vector<T, N> operator-(Vector<T, N> const& left, Vector<T, N> const& right);

	template<class T, std::size_t N>
	Vector<T, N>& operator-=(Vector<T, N>& left, Vector<T, N> const& right);

	template<class T1, std::convertible_to<T1> T2, std::size_t N>
	Vector<T1, N> operator*(Vector<T1, N> const& left, T2 const& right);

	template<class T1, std::convertible_to<T1> T2, std::size_t N>
	Vector<T1, N> operator*(T2 const& left, Vector<T1, N> const& right);

	template<class T, std::size_t N>
	Vector<T, N> operator*(Vector<T, N> const& left, Vector<T, N> const& right);

	template<class T1, std::convertible_to<T1> T2, std::size_t N>
	Vector<T1, N>& operator*=(Vector<T1, N>& left, T2 const& right);

	template<class T, std::size_t N>
	Vector<T, N>& operator*=(Vector<T, N>& left, Vector<T, N> const& right);

	template<class T1, std::convertible_to<T1> T2, std::size_t N>
	Vector<T1, N> operator/(Vector<T1, N> const& left, T2 const& right);

	template<class T1, std::convertible_to<T1> T2, std::size_t N>
	Vector<T1, N> operator/(T2 const& left, Vector<T1, N> const& right);

	template<class T, std::size_t N>
	Vector<T, N> operator/(Vector<T, N> const& left, Vector<T, N> const& right);

	template<class T1, std::convertible_to<T1> T2, std::size_t N>
	Vector<T1, N>& operator/=(Vector<T1, N>& left, T2 const& right);

	template<class T, std::size_t N>
	Vector<T, N>& operator/=(Vector<T, N>& left, Vector<T, N> const& right);

	template<class T, std::size_t N>
	bool operator==(Vector<T, N> const& left, Vector<T, N> const& right);

	template<class T, std::size_t N>
	bool operator!=(Vector<T, N> const& left, Vector<T, N> const& right);

	template<class T, std::size_t N>
	std::ostream& operator<<(std::ostream& stream, Vector<T, N> const& vector);

	using Vector2 = Vector<f32, 2>;
	using Vector3 = Vector<f32, 3>;
	using Vector4 = Vector<f32, 4>;

	using Vector2U = Vector<u32, 2>;
	using Vector3U = Vector<u32, 3>;
	using Vector4U = Vector<u32, 4>;

	using Vector2I = Vector<i32, 2>;
	using Vector3I = Vector<i32, 3>;
	using Vector4I = Vector<i32, 4>;


	template<class T, std::size_t N, std::size_t M> requires(N > 1 && M > 1)
		class Matrix
	{
	public:
		[[nodiscard]] T determinant() const requires(N == M);
		[[nodiscard]] Matrix<T, M, N> transpose() const;
		[[nodiscard]] Matrix<T, N, M> inverse() const requires(N == M);


		[[nodiscard]] Vector<T, M> const& operator[](size_t index) const;
		[[nodiscard]] Vector<T, M>& operator[](size_t index);
		[[nodiscard]] T const* get_data() const;


		[[nodiscard]] static Matrix<T, N, M> identity() requires (N == M);
		[[nodiscard]] static Matrix<T, 4, 4> translate(Vector<T, 3> const& vector) requires (N == 4 && M == 4);
		[[nodiscard]] static Matrix<T, 4, 4> scale(Vector<T, 3> const& vector) requires (N == 4 && M == 4);
		[[nodiscard]] static Matrix<T, 4, 4> look_at(Vector<T, 3> const& position, Vector<T, 3> const& target, Vector<T, 3> const& worldUp) requires(N == 4 && M == 4);
		[[nodiscard]] static Matrix<T, 4, 4> orthographic(T const& left, T const& right, T const& top, T const& bottom, T const& nearClipPlane, T const& farClipPlane);
		[[nodiscard]] static Matrix<T, 4, 4> perspective(T const& left, T const& right, T const& top, T const& bottom, T const& nearClipPlane, T const& farClipPlane) requires (N == 4 && M == 4);
		[[nodiscard]] static Matrix<T, 4, 4> perspective(T const& fov, T const& aspectRatio, T const& nearClipPlane, T const& farClipPlane) requires (N == 4 && M == 4);


		Matrix() = default;

		explicit Matrix(T const& value);

		template<std::convertible_to<T>... Args> requires(sizeof...(Args) == std::min(N, M))
			explicit(sizeof...(Args) <= 1) Matrix(Args const&... args);

		template<std::size_t K> requires(K == std::min(N, M))
			explicit Matrix(Vector<T, K> const& vec);

		template<std::convertible_to<T> ... Args> requires(sizeof...(Args) == N * M)
			explicit(sizeof...(Args) <= 1) Matrix(Args const&... args);

		template<std::size_t N1, std::size_t M1> requires (N1 < N&& M1 < M)
			explicit Matrix(Matrix<T, N1, M1> const& other);

		template<std::size_t N1, std::size_t M1> requires (N1 > N&& M1 > M)
			explicit Matrix(Matrix<T, N1, M1> const& other);

		Matrix(Matrix<T, N, M> const& other) = default;
		Matrix(Matrix<T, N, M>&& other) noexcept = default;

		~Matrix() = default;

		Matrix<T, N, M>& operator=(Matrix const& other) = default;
		Matrix<T, N, M>& operator=(Matrix&& other) noexcept = default;


	private:
		Vector<T, M> mData[N];
	};



	template<class T, std::size_t N, std::size_t M>
	Matrix<T, N, M> operator+(Matrix<T, N, M> const& left, Matrix<T, N, M> const& right);

	template<class T, std::size_t N, std::size_t M>
	Matrix<T, N, M>& operator+=(Matrix<T, N, M>& left, Matrix<T, N, M> const& right);

	template<class T, std::size_t N, std::size_t M>
	Matrix<T, N, M> operator-(Matrix<T, N, M> const& left, Matrix<T, N, M> const& right);

	template<class T, std::size_t N, std::size_t M>
	Matrix<T, N, M>& operator-=(Matrix<T, N, M>& left, Matrix<T, N, M> const& right);

	template<class T, std::size_t N, std::size_t M, std::convertible_to<T> T1>
	Matrix<T, N, M> operator*(Matrix<T, N, M> const& left, T1 const& right);

	template<class T, std::size_t N, std::size_t M, std::convertible_to<T> T1>
	Matrix<T, N, M> operator*(T1 const& left, Matrix<T, N, M> const& right);

	template<class T, std::size_t N, std::size_t M>
	Vector<T, N> operator*(Matrix<T, N, M> const& left, Vector<T, M> const& right);

	template<class T, std::size_t N, std::size_t M>
	Vector<T, M> operator*(Vector<T, N> const& left, Matrix<T, N, M> const& right);

	template<class T, std::size_t N, std::size_t M, std::size_t P>
	Matrix<T, N, P> operator*(Matrix<T, N, M> const& left, Matrix<T, M, P> const& right);

	template<class T, std::size_t N, std::size_t M, std::convertible_to<T> T1>
	Matrix<T, N, M>& operator*=(Matrix<T, N, M>& left, T1 const& right);

	template<class T, std::size_t N>
	Vector<T, N>& operator*=(Vector<T, N>& left, Matrix<T, N, N> const& right);

	template<class T, std::size_t N, std::size_t M>
	Matrix<T, N, M>& operator*=(Matrix<T, N, M>& left, Matrix<T, M, M> const& right);

	template<class T, std::size_t N, std::size_t M>
	std::ostream& operator<<(std::ostream& stream, Matrix<T, N, M> const& matrix);

	using Matrix2 = Matrix<f32, 2, 2>;
	using Matrix3 = Matrix<f32, 3, 3>;
	using Matrix4 = Matrix<f32, 4, 4>;


	struct Quaternion
	{
	public:
		f32 x;
		f32 y;
		f32 z;
		f32 w;


		LEOPPHAPI explicit Quaternion(f32 x = 0.0f, f32 y = 0.0f, f32 z = 0.0f, f32 w = 1.0f);
		LEOPPHAPI explicit Quaternion(Vector3 const& axis, f32 angleDegrees);

		[[nodiscard]] LEOPPHAPI Vector3 to_euler_angles() const;

		[[nodiscard]] LEOPPHAPI f32 get_norm() const;
		[[nodiscard]] LEOPPHAPI Quaternion normalized() const;
		LEOPPHAPI Quaternion& normalize();

		[[nodiscard]] LEOPPHAPI Quaternion conjugate() const;
		LEOPPHAPI Quaternion& conjugate_in_place();

		[[nodiscard]] LEOPPHAPI Quaternion inverse() const;
		LEOPPHAPI Quaternion& invert();

		[[nodiscard]] LEOPPHAPI explicit operator Matrix4() const;

		template<class T>
		[[nodiscard]] auto rotate(Vector<T, 3> const& vec) const;
	};


	[[nodiscard]] LEOPPHAPI Quaternion operator*(Quaternion const& left, Quaternion const& right);

	LEOPPHAPI Quaternion& operator*=(Quaternion& left, Quaternion const& right);

	LEOPPHAPI std::ostream& operator<<(std::ostream& os, Quaternion const& q);



	template<class T, std::size_t N> requires (N > 1)
		f32 Vector<T, N>::length() const
	{
		f32 squaredSum = 0;

		for (std::size_t i = 0; i < N; i++)
		{
			squaredSum += std::pow(mData[i], static_cast<T>(2));
		}

		return std::sqrt(squaredSum);
	}



	template<class T, std::size_t N> requires (N > 1)
		Vector<T, N> Vector<T, N>::normalized() const
	{
		return Vector<T, N>{*this}.normalize();
	}



	template<class T, std::size_t N> requires (N > 1)
		Vector<T, N>& Vector<T, N>::normalize()
	{
		if (auto const length = this->length(); length >= std::numeric_limits<f32>::epsilon())
		{
			for (std::size_t i = 0; i < N; i++)
			{
				mData[i] /= length;
			}
		}

		return *this;
	}



	template<class T, std::size_t N> requires (N > 1)
		T const& Vector<T, N>::operator[](size_t index) const
	{
		return get_element(this, index);
	}



	template<class T, std::size_t N> requires (N > 1)
		T& Vector<T, N>::operator[](size_t const index)
	{
		return get_element(this, index);
	}



	template<class T, std::size_t N> requires (N > 1)
		T const* Vector<T, N>::get_data() const
	{
		return mData;
	}



	template<class T, std::size_t N> requires (N > 1)
		Vector<T, N> Vector<T, N>::up()
	{
		Vector<T, N> ret;
		ret[1] = 1;
		return ret;
	}



	template<class T, std::size_t N> requires (N > 1)
		Vector<T, N> Vector<T, N>::down()
	{
		Vector<T, N> ret;
		ret[1] = -1;
		return ret;
	}



	template<class T, std::size_t N> requires (N > 1)
		Vector<T, N> Vector<T, N>::left()
	{
		Vector<T, N> ret;
		ret[0] = -1;
		return ret;
	}



	template<class T, std::size_t N> requires (N > 1)
		Vector<T, N> Vector<T, N>::right()
	{
		Vector<T, N> ret;
		ret[0] = 1;
		return ret;
	}



	template<class T, std::size_t N> requires (N > 1)
		Vector<T, N> Vector<T, N>::forward() requires (N >= 3)
	{
		Vector<T, N> ret;
		ret[2] = 1;
		return ret;
	}



	template<class T, std::size_t N> requires (N > 1)
		Vector<T, N> Vector<T, N>::backward() requires (N >= 3)
	{
		Vector<T, N> ret;
		ret[2] = -1;
		return ret;
	}



	template<class T, std::size_t N> requires (N > 1)
		template<std::convertible_to<T> T1>
	Vector<T, N>::Vector(T1 const& value)
	{
		for (std::size_t i = 0; i < N; i++)
		{
			mData[i] = static_cast<T>(value);
		}
	}



	template<class T, std::size_t N> requires (N > 1)
		template<std::convertible_to<T> ... Args> requires (sizeof...(Args) == N)
		Vector<T, N>::Vector(Args const&... args) :
		mData{static_cast<T>(args)...}
	{}



	template<class T, std::size_t N> requires (N > 1)
		template<std::size_t M> requires (M > N)
		Vector<T, N>::Vector(Vector<T, M> const& other)
	{
		for (std::size_t i = 0; i < N; i++)
		{
			mData[i] = other[i];
		}
	}



	template<class T, std::size_t N> requires (N > 1)
		template<std::size_t M> requires (M < N)
		Vector<T, N>::Vector(Vector<T, M> const& other, T const& fill)
	{
		for (std::size_t i = 0; i < M; i++)
		{
			mData[i] = other[i];
		}

		for (std::size_t i = M; i < N; ++i)
		{
			mData[i] = fill;
		}
	}



	template<class T, std::size_t N> requires (N > 1)
		decltype(auto) Vector<T, N>::get_element(auto* const self, std::size_t const index)
	{
		return self->mData[index];
	}



	template<class T, std::size_t N> requires (N > 1)
		T dot(Vector<T, N> const& left, Vector<T, N> const& right)
	{
		T ret{};

		for (size_t i = 0; i < N; i++)
		{
			ret += left[i] * right[i];
		}

		return ret;
	}



	template<class T, std::size_t N> requires (N > 1)
		Vector<T, N> cross(Vector<T, N> const& left, Vector<T, N> const& right) requires (N == 3)
	{
		return Vector<T, N>
		{
			left[1] * right[2] - left[2] * right[1],
				left[2] * right[0] - left[0] * right[2],
				left[0] * right[1] - left[1] * right[0]
		};
	}



	template<class T, std::size_t N> requires (N > 1)
		f32 distance(Vector<T, N> const& left, Vector<T, N> const& right)
	{
		f32 sum{0};

		for (size_t i = 0; i < N; i++)
		{
			sum += std::powf(static_cast<f32>(left[i] - right[i]), 2);
		}

		return std::sqrtf(sum);
	}



	template<class T, std::size_t N>
	Vector<T, N> lerp(Vector<T, N> const& from, Vector<T, N> const& to, float const t)
	{
		return (1 - t) * from + t * to;
	}



	template<class T, std::size_t N>
	Vector<T, N> operator-(Vector<T, N> const& operand)
	{
		Vector<T, N> ret;
		for (size_t i = 0; i < N; i++)
		{
			ret[i] = -operand[i];
		}
		return ret;
	}



	template<class T, std::size_t N>
	Vector<T, N> operator+(Vector<T, N> const& left, Vector<T, N> const& right)
	{
		Vector<T, N> ret;
		for (size_t i = 0; i < N; i++)
		{
			ret[i] = left[i] + right[i];
		}
		return ret;
	}



	template<class T, std::size_t N>
	Vector<T, N>& operator+=(Vector<T, N>& left, Vector<T, N> const& right)
	{
		for (std::size_t i = 0; i < N; i++)
		{
			left[i] += right[i];
		}
		return left;
	}



	template<class T, std::size_t N>
	Vector<T, N> operator-(Vector<T, N> const& left, Vector<T, N> const& right)
	{
		Vector<T, N> ret;
		for (std::size_t i = 0; i < N; i++)
		{
			ret[i] = left[i] - right[i];
		}
		return ret;
	}



	template<class T, std::size_t N>
	Vector<T, N>& operator-=(Vector<T, N>& left, Vector<T, N> const& right)
	{
		for (std::size_t i = 0; i < N; i++)
		{
			left[i] -= right[i];
		}
		return left;
	}



	template<class T1, std::convertible_to<T1> T2, std::size_t N>
	Vector<T1, N> operator*(Vector<T1, N> const& left, T2 const& right)
	{
		Vector<T1, N> ret;
		for (size_t i = 0; i < N; i++)
		{
			ret[i] = left[i] * static_cast<T1>(right);
		}
		return ret;
	}



	template<class T1, std::convertible_to<T1> T2, std::size_t N>
	Vector<T1, N> operator*(T2 const& left, Vector<T1, N> const& right)
	{
		Vector<T1, N> ret;
		for (std::size_t i = 0; i < N; i++)
		{
			ret[i] = static_cast<T1>(left) * right[i];
		}
		return ret;
	}



	template<class T, std::size_t N>
	Vector<T, N> operator*(Vector<T, N> const& left, Vector<T, N> const& right)
	{
		Vector<T, N> ret;
		for (std::size_t i = 0; i < N; i++)
		{
			ret[i] = left[i] * right[i];
		}
		return ret;
	}



	template<class T1, std::convertible_to<T1> T2, std::size_t N>
	Vector<T1, N>& operator*=(Vector<T1, N>& left, T2 const& right)
	{
		for (std::size_t i = 0; i < N; i++)
		{
			left[i] *= static_cast<T1>(right);
		}
		return left;
	}



	template<class T, std::size_t N>
	Vector<T, N>& operator*=(Vector<T, N>& left, Vector<T, N> const& right)
	{
		for (std::size_t i = 0; i < N; i++)
		{
			left[i] *= right[i];
		}
		return left;
	}



	template<class T1, std::convertible_to<T1> T2, std::size_t N>
	Vector<T1, N> operator/(Vector<T1, N> const& left, T2 const& right)
	{
		Vector<T1, N> ret;
		for (std::size_t i = 0; i < N; i++)
		{
			ret[i] = left[i] / static_cast<T1>(right);
		}
		return ret;
	}



	template<class T1, std::convertible_to<T1> T2, std::size_t N>
	Vector<T1, N> operator/(T2 const& left, Vector<T1, N> const& right)
	{
		Vector<T1, N> ret;
		T1 const numerator{static_cast<T1>(left)};
		for (std::size_t i = 0; i < N; i++)
		{
			ret[i] = numerator / right[i];
		}
		return ret;
	}



	template<class T, std::size_t N>
	Vector<T, N> operator/(Vector<T, N> const& left, Vector<T, N> const& right)
	{
		Vector<T, N> ret;
		for (std::size_t i = 0; i < N; i++)
		{
			ret[i] = left[i] / right[i];
		}
		return ret;
	}



	template<class T1, std::convertible_to<T1> T2, std::size_t N>
	Vector<T1, N>& operator/=(Vector<T1, N>& left, T2 const& right)
	{
		for (std::size_t i = 0; i < N; i++)
		{
			left[i] /= static_cast<T1>(right);
		}
		return left;
	}



	template<class T, std::size_t N>
	Vector<T, N>& operator/=(Vector<T, N>& left, Vector<T, N> const& right)
	{
		for (std::size_t i = 0; i < N; i++)
		{
			left[i] /= right[i];
		}
		return left;
	}



	template<class T, std::size_t N>
	bool operator==(Vector<T, N> const& left, Vector<T, N> const& right)
	{
		for (size_t i = 0; i < N; i++)
		{
			if (left[i] != right[i])
			{
				return false;
			}
		}
		return true;
	}



	template<class T, std::size_t N>
	bool operator!=(Vector<T, N> const& left, Vector<T, N> const& right)
	{
		return !(left == right);
	}



	template<class T, std::size_t N>
	std::ostream& operator<<(std::ostream& stream, Vector<T, N> const& vector)
	{
		stream << "(";
		for (size_t i = 0; i < N; i++)
		{
			stream << vector[i];
			if (i != N - 1)
			{
				stream << ", ";
			}
		}
		stream << ")";
		return stream;
	}



	template<class T, std::size_t N, std::size_t M> requires (N > 1 && M > 1)
		T Matrix<T, N, M>::determinant() const requires (N == M)
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



	template<class T, std::size_t N, std::size_t M> requires (N > 1 && M > 1)
		Matrix<T, M, N> Matrix<T, N, M>::transpose() const
	{
		Matrix<T, M, N> ret;
		for (size_t i = 0; i < N; i++)
		{
			for (size_t j = 0; j < M; j++)
			{
				ret[j][i] = mData[i][j];
			}
		}
		return ret;
	}



	template<class T, std::size_t N, std::size_t M> requires (N > 1 && M > 1)
		Matrix<T, N, M> Matrix<T, N, M>::inverse() const requires (N == M)
	{
		Matrix<T, N, M> left{*this};
		Matrix<T, N, M> right{Matrix<T, N, M>::identity()};

		// Iterate over the main diagonal/submatrices
		for (std::size_t i = 0; i < N; i++)
		{
			// Index of the row with the largest absolute value element in the ith column
			auto pivotInd{i};

			// Find index of the row with the largest absolute value element
			for (auto j = pivotInd + 1; j < N; j++)
			{
				if (std::abs(left[j][i]) > std::abs(left[pivotInd][i]))
				{
					pivotInd = j;
				}
			}

			// swap pivot row with the ith
			auto tmp{left[i]};
			left[i] = left[pivotInd];
			left[pivotInd] = tmp;

			tmp = right[i];
			right[i] = right[pivotInd];
			right[pivotInd] = tmp;

			// If the main diagonal element is non-zero
			// 1. Normalize the row so that the element is 1
			// 2. Reduce all rows below so that its elements under are zero
			// If the main diagonal element is zero, this will produce NAN, but that's fine
			// Because then zero is the number with the highest absolute value in the column
			// So all other elements are also zero in the column
			// The matrix is singular and we return garbage
			auto const div{left[i][i]};
			left[i] /= div;
			right[i] /= div;

			for (auto j = i + 1; j < N; j++)
			{
				auto const mult{left[j][i]}; // Theoretically left[i][i] is 1 so the multiplier is the element itself
				left[j] -= mult * left[i];
				right[j] -= mult * right[i];
			}
		}

		// Left is in reduced echelon form
		// Now eliminate the remaining non-zeros outside the main diagonal
		for (std::size_t i = 0; i < N; i++)
		{
			for (auto j = i + 1; j < N; j++)
			{
				// We subtract the jth row from the ith one
				// Because it is guaranteed to have 0s before the main diagonal
				// And thus it won't mess up the element in the ith row before the jth element
				auto const mult{left[i][j]}; // left[j][j] is 1 so the multiplier is the element itself
				left[i] -= mult * left[j];
				right[i] -= mult * right[j];
			}
		}
		return right;
	}



	template<class T, std::size_t N, std::size_t M> requires (N > 1 && M > 1)
		Vector<T, M> const& Matrix<T, N, M>::operator[](size_t const index) const
	{
		return mData[index];
	}



	template<class T, std::size_t N, std::size_t M> requires (N > 1 && M > 1)
		Vector<T, M>& Matrix<T, N, M>::operator[](size_t const index)
	{
		return mData[index];
	}



	template<class T, std::size_t N, std::size_t M> requires (N > 1 && M > 1)
		T const* Matrix<T, N, M>::get_data() const
	{
		return mData;
	}



	template<class T, std::size_t N, std::size_t M> requires (N > 1 && M > 1)
		Matrix<T, N, M> Matrix<T, N, M>::identity() requires (N == M)
	{
		return Matrix<T, N, M>{1};
	}



	template<class T, std::size_t N, std::size_t M> requires (N > 1 && M > 1)
		Matrix<T, 4, 4> Matrix<T, N, M>::translate(Vector<T, 3> const& vector) requires (N == 4 && M == 4)
	{
		Matrix<T, 4, 4> ret = identity();
		for (size_t i = 0; i < 3; i++)
		{
			ret[3][i] = vector[i];
		}
		return ret;
	}



	template<class T, std::size_t N, std::size_t M> requires (N > 1 && M > 1)
		Matrix<T, 4, 4> Matrix<T, N, M>::scale(Vector<T, 3> const& vector) requires (N == 4 && M == 4)
	{
		Matrix<T, 4, 4> ret = identity();
		for (size_t i = 0; i < 3; i++)
		{
			ret[i][i] = vector[i];
		}
		return ret;
	}



	template<class T, std::size_t N, std::size_t M> requires (N > 1 && M > 1)
		Matrix<T, 4, 4> Matrix<T, N, M>::look_at(Vector<T, 3> const& position, Vector<T, 3> const& target, Vector<T, 3> const& worldUp) requires (N == 4 && M == 4)
	{
		Vector<T, 3> z{(target - position).normalized()};
		Vector<T, 3> x{cross(worldUp, z).normalized()};
		Vector<T, 3> y{cross(z, x)};
		return Matrix<T, 4, 4>
		{
			x[0], y[0], z[0], 0,
				x[1], y[1], z[1], 0,
				x[2], y[2], z[2], 0,
				-dot(x, position), -dot(y, position), -dot(z, position), 1
		};
	}



	template<class T, std::size_t N, std::size_t M> requires (N > 1 && M > 1)
		Matrix<T, 4, 4> Matrix<T, N, M>::orthographic(T const& left, T const& right, T const& top, T const& bottom, T const& nearClipPlane, T const& farClipPlane)
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



	template<class T, std::size_t N, std::size_t M> requires (N > 1 && M > 1)
		Matrix<T, 4, 4> Matrix<T, N, M>::perspective(T const& fov, T const& aspectRatio, T const& nearClipPlane, T const& farClipPlane) requires (N == 4 && M == 4)
	{
		T tanHalfFov{static_cast<T>(std::tan(fov / static_cast<T>(2)))};
		T top{nearClipPlane * tanHalfFov};
		T bottom{-top};
		T right{top * aspectRatio};
		T left{-right};
		return perspective(left, right, top, bottom, nearClipPlane, farClipPlane);
	}



	template<class T, std::size_t N, std::size_t M> requires (N > 1 && M > 1)
		Matrix<T, 4, 4> Matrix<T, N, M>::perspective(T const& left, T const& right, T const& top, T const& bottom, T const& nearClipPlane, T const& farClipPlane) requires (N == 4 && M == 4)
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



	template<class T, std::size_t N, std::size_t M> requires (N > 1 && M > 1)
		Matrix<T, N, M>::Matrix(T const& value)
	{
		for (size_t i = 0; i < N && i < M; i++)
		{
			mData[i][i] = value;
		}
	}



	template<class T, std::size_t N, std::size_t M> requires (N > 1 && M > 1)
		template<std::size_t K> requires (K == std::min(N, M))
		Matrix<T, N, M>::Matrix(Vector<T, K> const& vec)
	{
		for (size_t i = 0; i < K; i++)
		{
			mData[i][i] = vec[i];
		}
	}



	template<class T, std::size_t N, std::size_t M> requires (N > 1 && M > 1)
		template<std::convertible_to<T> ... Args> requires (sizeof...(Args) == N * M)
		Matrix<T, N, M>::Matrix(Args const&... args)
	{
		T argArr[M * N]{static_cast<T>(args)...};
		for (size_t i = 0; i < N; i++)
		{
			for (size_t j = 0; j < M; j++)
			{
				mData[i][j] = argArr[i * M + j];
			}
		}
	}



	template<class T, std::size_t N, std::size_t M> requires (N > 1 && M > 1)
		template<std::size_t N1, std::size_t M1> requires (N1 < N&& M1 < M)
		Matrix<T, N, M>::Matrix(Matrix<T, N1, M1> const& other)
	{
		for (std::size_t i = 0; i < N1; i++)
		{
			mData[i] = Vector<T, M>{other[i], 0};
		}

		for (std::size_t i = N1; i < N; ++i)
		{
			mData[i] = Vector<T, M>{0};
		}

		mData[N - 1][M - 1] = static_cast<T>(1);
	}



	template<class T, std::size_t N, std::size_t M> requires (N > 1 && M > 1)
		template<std::size_t N1, std::size_t M1> requires (N1 > N&& M1 > M)
		Matrix<T, N, M>::Matrix(Matrix<T, N1, M1> const& other)
	{
		for (std::size_t i = 0; i < N; i++)
		{
			mData[i] = Vector<T, M>{other[i]};
		}
	}



	template<class T, std::size_t N, std::size_t M> requires (N > 1 && M > 1)
		template<std::convertible_to<T> ... Args> requires (sizeof...(Args) == std::min(N, M))
		Matrix<T, N, M>::Matrix(Args const&... args)
	{
		T argArr[M > N ? N : M]{static_cast<T>(args)...};
		for (size_t i = 0; i < M && i < N; i++)
		{
			mData[i][i] = argArr[i];
		}
	}



	template<class T, std::size_t N, std::size_t M>
	Matrix<T, N, M> operator+(Matrix<T, N, M> const& left, Matrix<T, N, M> const& right)
	{
		Matrix<T, N, M> ret;
		for (std::size_t i = 0; i < N; i++)
		{
			ret[i] = left[i] + right[i];
		}
		return ret;
	}



	template<class T, std::size_t N, std::size_t M>
	Matrix<T, N, M>& operator+=(Matrix<T, N, M>& left, Matrix<T, N, M> const& right)
	{
		for (std::size_t i = 0; i < N; i++)
		{
			left[i] += right[i];
		}
		return left;
	}



	template<class T, std::size_t N, std::size_t M>
	Matrix<T, N, M> operator-(Matrix<T, N, M> const& left, Matrix<T, N, M> const& right)
	{
		Matrix<T, N, M> ret;
		for (std::size_t i = 0; i < N; i++)
		{
			ret[i] = left[i] - right[i];
		}
		return ret;
	}



	template<class T, std::size_t N, std::size_t M>
	Matrix<T, N, M>& operator-=(Matrix<T, N, M>& left, Matrix<T, N, M> const& right)
	{
		for (std::size_t i = 0; i < N; i++)
		{
			left[i] -= right[i];
		}
		return left;
	}



	template<class T, std::size_t N, std::size_t M, std::convertible_to<T> T1>
	Matrix<T, N, M> operator*(Matrix<T, N, M> const& left, T1 const& right)
	{
		Matrix<T, N, M> ret;
		for (std::size_t i = 0; i < N; i++)
		{
			ret[i] = left[i] * static_cast<T>(right);
		}
		return ret;
	}



	template<class T, std::size_t N, std::size_t M, std::convertible_to<T> T1>
	Matrix<T, N, M> operator*(T1 const& left, Matrix<T, N, M> const& right)
	{
		Matrix<T, N, M> ret;
		for (std::size_t i = 0; i < N; i++)
		{
			ret[i] = static_cast<T>(left) * right[i];
		}
		return ret;
	}



	template<class T, std::size_t N, std::size_t M>
	Vector<T, N> operator*(Matrix<T, N, M> const& left, Vector<T, M> const& right)
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
	Vector<T, M> operator*(Vector<T, N> const& left, Matrix<T, N, M> const& right)
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
	Matrix<T, N, P> operator*(Matrix<T, N, M> const& left, Matrix<T, M, P> const& right)
	{
		Matrix<T, N, P> ret;
		for (size_t i = 0; i < N; i++)
		{
			for (size_t j = 0; j < P; j++)
			{
				for (size_t k = 0; k < M; k++)
				{
					ret[i][j] += left[i][k] * right[k][j];
				}
			}
		}
		return ret;
	}



	template<class T, std::size_t N, std::size_t M, std::convertible_to<T> T1>
	Matrix<T, N, M>& operator*=(Matrix<T, N, M>& left, T1 const& right)
	{
		for (std::size_t i = 0; i < N; i++)
		{
			left[i] *= static_cast<T>(right);
		}
		return left;
	}



	template<class T, std::size_t N>
	Vector<T, N>& operator*=(Vector<T, N>& left, Matrix<T, N, N> const& right)
	{
		return left = left * right;
	}



	template<class T, std::size_t N, std::size_t M>
	Matrix<T, N, M>& operator*=(Matrix<T, N, M>& left, Matrix<T, M, M> const& right)
	{
		return left = left * right;
	}



	template<class T, std::size_t N, std::size_t M>
	std::ostream& operator<<(std::ostream& stream, Matrix<T, N, M> const& matrix)
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



	template<class T>
	auto Quaternion::rotate(Vector<T, 3> const& vec) const
	{
		auto const retQuat{*this * Quaternion{0, vec[0], vec[1], vec[2]} *conjugate()};
		return Vector<T, 3>{retQuat.x, retQuat.y, retQuat.z};
	}
}
