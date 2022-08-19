#pragma once

#include "Types.hpp"

#include <array>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <limits>
#include <ostream>


namespace leopph
{
	template<class T, std::size_t N> requires(N > 1)
	class Vector
	{
		public:
			[[nodiscard]] f32 length() const;
			[[nodiscard]] Vector<T, N> normalized() const;
			Vector<T, N>& normalize();


			[[nodiscard]] T const& operator[](size_t index) const;
			[[nodiscard]] T& operator[](size_t index);
			[[nodiscard]] std::array<T, N> const& get_data() const;


			[[nodiscard]] static Vector<T, N> up();
			[[nodiscard]] static Vector<T, N> down();
			[[nodiscard]] static Vector<T, N> left();
			[[nodiscard]] static Vector<T, N> right();
			[[nodiscard]] static Vector<T, N> forward() requires(N >= 3);
			[[nodiscard]] static Vector<T, N> backward() requires(N >= 3);


			Vector() = default;

			explicit Vector(T const& value);

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

			std::array<T, N> mData{};
	};


	using Vector2 = Vector<f32, 2>;
	using Vector3 = Vector<f32, 3>;
	using Vector4 = Vector<f32, 4>;

	using Vector2U = Vector<u32, 2>;
	using Vector3U = Vector<u32, 3>;
	using Vector4U = Vector<u32, 4>;

	using Vector2I = Vector<i32, 2>;
	using Vector3I = Vector<i32, 3>;
	using Vector4I = Vector<i32, 4>;


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
	std::array<T, N> const& Vector<T, N>::get_data() const
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
	Vector<T, N>::Vector(T const& value)
	{
		mData.fill(value);
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
}
