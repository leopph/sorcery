#pragma once

#include "Types.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <limits>
#include <numeric>
#include <ostream>


namespace leopph
{
	template<class T, std::size_t N> requires(N > 1)
	class Vector
	{
		public:
			[[nodiscard]] float get_length() const;
			[[nodiscard]] Vector<T, N> normalized() const;
			Vector<T, N>& normalize();


			[[nodiscard]] auto& operator[](size_t index) const;
			[[nodiscard]] auto& operator[](size_t index);
			[[nodiscard]] auto& get_data() const;


			[[nodiscard]] static T dot(Vector<T, N> const& left, Vector<T, N> const& right);
			[[nodiscard]] static Vector<T, N> cross(Vector<T, N> const& left, Vector<T, N> const& right) requires(N == 3);
			[[nodiscard]] static float distance(Vector<T, N> const& left, Vector<T, N> const& right);


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

			template<std::size_t M> requires (N > M)
			explicit Vector(Vector<T, M> const& other, T const& fillVal = 1);

			Vector(Vector<T, N> const& other) = default;
			Vector(Vector<T, N>&& other) noexcept = default;

			Vector<T, N>& operator=(Vector<T, N> const& other) = default;
			Vector<T, N>& operator=(Vector<T, N>&& other) noexcept = default;

			~Vector() = default;

		private:
			[[nodiscard]] static decltype(auto) get_element_common(auto* self, std::size_t index);

			std::array<T, N> mData{};
	};


	// Definitions



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
	auto& Vector<T, N>::get_data() const
	{
		return mData;
	}



	template<class T, std::size_t N> requires (N > 1)
	auto& Vector<T, N>::operator[](size_t index) const
	{
		return get_element_common(this, index);
	}



	template<class T, std::size_t N> requires (N > 1)
	auto& Vector<T, N>::operator[](size_t const index)
	{
		return get_element_common(this, index);
	}



	template<class T, std::size_t N> requires (N > 1)
	float Vector<T, N>::get_length() const
	{
		return std::sqrtf(
			static_cast<float>(
				std::accumulate(mData.begin(), mData.end(), static_cast<T>(0), [](T const& sum, T const& elem)
				{
					return sum + std::powf(elem, 2);
				})));
	}



	template<class T, std::size_t N> requires (N > 1)
	Vector<T, N> Vector<T, N>::normalized() const
	{
		return Vector<T, N>{*this}.normalize();
	}



	template<class T, std::size_t N> requires (N > 1)
	Vector<T, N>& Vector<T, N>::normalize()
	{
		if (auto length = get_length(); std::abs(length) >= std::numeric_limits<float>::epsilon())
		{
			std::for_each(mData.begin(), mData.end(), [length](T& elem)
			{
				elem /= static_cast<T>(length);
			});
		}
		return *this;
	}



	template<class T, std::size_t N> requires (N > 1)
	T Vector<T, N>::dot(Vector<T, N> const& left, Vector<T, N> const& right)
	{
		T ret{};
		for (size_t i = 0; i < N; i++)
		{
			ret += left[i] * right[i];
		}
		return ret;
	}



	template<class T, std::size_t N> requires (N > 1)
	Vector<T, N> Vector<T, N>::cross(Vector<T, N> const& left, Vector<T, N> const& right) requires (N == 3)
	{
		return Vector<T, N>
		{
			left[1] * right[2] - left[2] * right[1],
			left[2] * right[0] - left[0] * right[2],
			left[0] * right[1] - left[1] * right[0]
		};
	}



	template<class T, std::size_t N> requires (N > 1)
	float Vector<T, N>::distance(Vector<T, N> const& left, Vector<T, N> const& right)
	{
		auto sum{0.f};
		for (size_t i = 0; i < N; i++)
		{
			sum += std::powf(static_cast<float>(left[i] - right[i]), 2);
		}
		return std::sqrtf(sum);
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
		std::ranges::copy_n(other.get_data().begin(), N, mData.begin());
	}



	template<class T, std::size_t N> requires (N > 1)
	template<std::size_t M> requires (N > M)
	Vector<T, N>::Vector(Vector<T, M> const& other, T const& fillVal)
	{
		std::ranges::copy(other.get_data(), mData.begin());
		std::ranges::fill(mData.begin() + M, mData.end(), fillVal);
	}



	template<class T, std::size_t N> requires (N > 1)
	decltype(auto) Vector<T, N>::get_element_common(auto* const self, std::size_t const index)
	{
		return self->mData[index];
	}



	// Non-member operators

	// Returns a Vector that's components are the additives inverses of this Vector's components.
	template<class T, std::size_t N>
	Vector<T, N> operator-(Vector<T, N> const& operand) noexcept
	{
		Vector<T, N> ret;
		for (size_t i = 0; i < N; i++)
		{
			ret[i] = -(operand[i]);
		}
		return ret;
	}



	// Returns the sum of the input Vectors.
	template<class T, std::size_t N>
	Vector<T, N> operator+(Vector<T, N> const& left, Vector<T, N> const& right) noexcept
	{
		Vector<T, N> ret;
		for (size_t i = 0; i < N; i++)
		{
			ret[i] = left[i] + right[i];
		}
		return ret;
	}



	// Sets the left operand to the sum of the input Vectors.
	// Returns a reference to the left operand.
	template<class T, std::size_t N>
	Vector<T, N>& operator+=(Vector<T, N>& left, Vector<T, N> const& right) noexcept
	{
		for (std::size_t i = 0; i < N; i++)
		{
			left[i] += right[i];
		}
		return left;
	}



	// Returns the difference of the input Vectors.
	template<class T, std::size_t N>
	Vector<T, N> operator-(Vector<T, N> const& left, Vector<T, N> const& right) noexcept
	{
		Vector<T, N> ret;
		for (std::size_t i = 0; i < N; i++)
		{
			ret[i] = left[i] - right[i];
		}
		return ret;
	}



	// Sets the left operand to the difference of the input Vectors.
	// Returns a reference to the left operand.
	template<class T, std::size_t N>
	Vector<T, N>& operator-=(Vector<T, N>& left, Vector<T, N> const& right) noexcept
	{
		for (std::size_t i = 0; i < N; i++)
		{
			left[i] -= right[i];
		}
		return left;
	}



	// Returns the result of the scalar multiplication of the input values.
	template<class T1, std::convertible_to<T1> T2, std::size_t N>
	Vector<T1, N> operator*(Vector<T1, N> const& left, T2 const& right) noexcept
	{
		Vector<T1, N> ret;
		for (size_t i = 0; i < N; i++)
		{
			ret[i] = left[i] * static_cast<T1>(right);
		}
		return ret;
	}



	// Returns the result of the scalar multiplication of the input values.
	template<class T1, std::convertible_to<T1> T2, std::size_t N>
	Vector<T1, N> operator*(T2 const& left, Vector<T1, N> const& right) noexcept
	{
		Vector<T1, N> ret;
		for (std::size_t i = 0; i < N; i++)
		{
			ret[i] = static_cast<T1>(left) * right[i];
		}
		return ret;
	}



	// Returns the component-wise product of the input Vectors.
	template<class T, std::size_t N>
	Vector<T, N> operator*(Vector<T, N> const& left, Vector<T, N> const& right) noexcept
	{
		Vector<T, N> ret;
		for (std::size_t i = 0; i < N; i++)
		{
			ret[i] = left[i] * right[i];
		}
		return ret;
	}



	// Sets the left operand to the result of the scalar multiplication of the input values.
	// Returns a reference to the left operand.
	template<class T1, std::convertible_to<T1> T2, std::size_t N>
	Vector<T1, N>& operator*=(Vector<T1, N>& left, T2 const& right) noexcept
	{
		for (std::size_t i = 0; i < N; i++)
		{
			left[i] *= static_cast<T1>(right);
		}
		return left;
	}



	// Sets the left oparend to the component-wise product of the input Vectors.
	// Returns a reference to the left operand.
	template<class T, std::size_t N>
	Vector<T, N>& operator*=(Vector<T, N>& left, Vector<T, N> const& right) noexcept
	{
		for (std::size_t i = 0; i < N; i++)
		{
			left[i] *= right[i];
		}
		return left;
	}



	// Returns the result of the scalar division of the input values.
	template<class T1, std::convertible_to<T1> T2, std::size_t N>
	Vector<T1, N> operator/(Vector<T1, N> const& left, T2 const& right) noexcept
	{
		Vector<T1, N> ret;
		for (std::size_t i = 0; i < N; i++)
		{
			ret[i] = left[i] / static_cast<T1>(right);
		}
		return ret;
	}



	// Returns the result of the scalar division of the input values.
	template<class T1, std::convertible_to<T1> T2, std::size_t N>
	Vector<T1, N> operator/(T2 const& left, Vector<T1, N> const& right) noexcept
	{
		Vector<T1, N> ret;
		T1 const numerator{static_cast<T1>(left)};
		for (std::size_t i = 0; i < N; i++)
		{
			ret[i] = numerator / right[i];
		}
		return ret;
	}



	// Returns the component-wise quotient of the input Vectors.
	template<class T, std::size_t N>
	Vector<T, N> operator/(Vector<T, N> const& left, Vector<T, N> const& right) noexcept
	{
		Vector<T, N> ret;
		for (std::size_t i = 0; i < N; i++)
		{
			ret[i] = left[i] / right[i];
		}
		return ret;
	}



	// Sets the left operand to the result of the scalar division of the input values.
	// Returns a reference to the left operand.
	template<class T1, std::convertible_to<T1> T2, std::size_t N>
	Vector<T1, N>& operator/=(Vector<T1, N>& left, T2 const& right) noexcept
	{
		for (std::size_t i = 0; i < N; i++)
		{
			left[i] /= static_cast<T1>(right);
		}
		return left;
	}



	// Sets the left operand to the component-wise quotient of the input Vectors.
	// Returns a reference to the left operand.
	template<class T, std::size_t N>
	Vector<T, N>& operator/=(Vector<T, N>& left, Vector<T, N> const& right) noexcept
	{
		for (std::size_t i = 0; i < N; i++)
		{
			left[i] /= right[i];
		}
		return left;
	}



	// Returns whether the input Vectors are equal.
	template<class T, std::size_t N>
	bool operator==(Vector<T, N> const& left, Vector<T, N> const& right) noexcept
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



	// Returns whether the input Vectors are not equal.
	template<class T, std::size_t N>
	bool operator!=(Vector<T, N> const& left, Vector<T, N> const& right) noexcept
	{
		return !(left == right);
	}



	// Prints the input Vector on the specified output stream.
	template<class T, std::size_t N>
	std::ostream& operator<<(std::ostream& stream, Vector<T, N> const& vector) noexcept
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



	// Free functions for Vector
	namespace math
	{
		template<class T, std::size_t N>
		Vector<T, N> lerp(Vector<T, N> const& from, Vector<T, N> const& to, float const t)
		{
			return (1 - t) * from + t * to;
		}
	}


	using Vector2 = Vector<f32, 2>;
	using Vector3 = Vector<f32, 3>;
	using Vector4 = Vector<f32, 4>;

	using Vector2U = Vector<u32, 2>;
	using Vector3U = Vector<u32, 3>;
	using Vector4U = Vector<u32, 4>;

	using Vector2I = Vector<i32, 2>;
	using Vector3I = Vector<i32, 3>;
	using Vector4I = Vector<i32, 4>;
}
