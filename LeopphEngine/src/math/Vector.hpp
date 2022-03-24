#pragma once

#include "Math.hpp"

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
	namespace internal
	{
		// Template for vectors of different types and dimensions.
		// Vectors are row/column agnostic. The interpretation depends on the context.
		// Should not be instantiated explicitly unless necessary.
		template<class T, std::size_t N>
			requires(N > 1)
		class Vector
		{
			public:
				// Creates a Vector with all components set to 0.
				constexpr Vector() = default;

				// Creates a Vector with all components set to the input value.
				constexpr explicit Vector(T const& value) noexcept;

				// Creates a Vector with components set to the input values.
				template<std::convertible_to<T>... Args>
					requires(sizeof...(Args) == N)
				constexpr explicit(sizeof...(Args) <= 1) Vector(Args const&... args) noexcept;

				// Creates a Vector from a higher dimensional one by dropping its last components.
				template<std::size_t M>
					requires (M > N)
				constexpr explicit Vector(Vector<T, M> const& other) noexcept;

				// Creates a Vector from a lower dimensional one and fills the remaining components with the passed scalar.
				template<std::size_t M>
					requires (N > M)
				constexpr explicit Vector(Vector<T, M> const& other, T const& fillVal = 1) noexcept;

				constexpr Vector(Vector<T, N> const& other) = default;
				constexpr auto operator=(Vector<T, N> const& other) -> Vector<T, N>& = default;

				constexpr Vector(Vector<T, N>&& other) = default;
				constexpr auto operator=(Vector<T, N>&& other) -> Vector<T, N>& = default;

				constexpr ~Vector() = default;

				// Creates a Vector with its second component set to 1 and all other components set to 0.
				[[nodiscard]] constexpr static auto Up() noexcept -> Vector<T, N>;

				// Creates a Vector with its second component set to -1 and all other components set to 0.
				[[nodiscard]] constexpr static auto Down() noexcept -> Vector<T, N>;

				// Creates a Vector with its first component set to -1 and all other components set to 0.
				[[nodiscard]] constexpr static auto Left() noexcept -> Vector<T, N>;

				// Creates a Vector with its first component set to 1 and all other components set to 0.
				[[nodiscard]] constexpr static auto Right() noexcept -> Vector<T, N>;

				// Creates a Vector with its third component set to 1 and all other components set to 0.
				[[nodiscard]] constexpr static auto Forward() noexcept -> Vector<T, N>
					requires(N >= 3);

				// Creates a Vector with its third component set to -1 and all other components set to 0.
				[[nodiscard]] constexpr static auto Backward() noexcept -> Vector<T, N>
					requires(N >= 3);

				// Returns a reference to the internal data structure.
				[[nodiscard]] constexpr auto Data() const noexcept -> auto&;

				// Get the Vector's components.
				// Indexes are not bounds-checked.
				[[nodiscard]] constexpr auto operator[](size_t index) const noexcept -> auto&;

				// Get the Vector's components.
				// Indexes are not bounds-checked. 
				[[nodiscard]] constexpr auto operator[](size_t index) noexcept -> auto&;

				// Get the length of this Vector.
				[[nodiscard]] auto Length() const noexcept -> float;

				// Returns a Vector that has the same direction as this Vector, but has a length of 1.
				[[nodiscard]] auto Normalized() const noexcept -> Vector<T, N>;

				// Changes this Vector so that it has the same direction, but a length of 1.
				// Returns a reference to this Vector.
				auto Normalize() noexcept -> Vector<T, N>&;

				// Returns the dot product of the input Vectors.
				[[nodiscard]] constexpr static auto Dot(Vector<T, N> const& left, Vector<T, N> const& right) noexcept -> T;

				// Returns the cross product of the input Vectors.
				[[nodiscard]] constexpr static auto Cross(Vector<T, N> const& left, Vector<T, N> const& right) noexcept -> Vector<T, N>
					requires(N == 3);

				// Returns the Euclidean distance of the input Vectors.
				[[nodiscard]] static auto Distance(Vector<T, N> const& left, Vector<T, N> const& right) noexcept -> float;

			private:
				// Helper function to get const and non-const references to elements depending on context.
				[[nodiscard]] constexpr static auto GetElementCommon(auto* self, std::size_t index) -> decltype(auto);

				std::array<T, N> m_Data{};
		};


		// Definitions

		template<class T, std::size_t N>
			requires (N > 1)
		constexpr Vector<T, N>::Vector(T const& value) noexcept
		{
			m_Data.fill(value);
		}


		template<class T, std::size_t N>
			requires (N > 1)
		template<std::convertible_to<T> ... Args>
			requires (sizeof...(Args) == N)
		constexpr Vector<T, N>::Vector(Args const&... args) noexcept :
			m_Data{static_cast<T>(args)...}
		{}


		template<class T, std::size_t N>
			requires (N > 1)
		template<std::size_t M>
			requires (M > N)
		constexpr Vector<T, N>::Vector(Vector<T, M> const& other) noexcept
		{
			std::ranges::copy_n(other.Data().begin(), N, m_Data.begin());
		}


		template<class T, std::size_t N>
			requires (N > 1)
		template<std::size_t M>
			requires (N > M)
		constexpr Vector<T, N>::Vector(Vector<T, M> const& other, T const& fillVal) noexcept
		{
			std::ranges::copy(other.Data(), m_Data.begin());
			std::ranges::fill(m_Data.begin() + M, m_Data.end(), fillVal);
		}


		template<class T, std::size_t N>
			requires (N > 1)
		constexpr auto Vector<T, N>::Up() noexcept -> Vector<T, N>
		{
			Vector<T, N> ret;
			ret[1] = 1;
			return ret;
		}


		template<class T, std::size_t N>
			requires (N > 1)
		constexpr auto Vector<T, N>::Down() noexcept -> Vector<T, N>
		{
			Vector<T, N> ret;
			ret[1] = -1;
			return ret;
		}


		template<class T, std::size_t N>
			requires (N > 1)
		constexpr auto Vector<T, N>::Left() noexcept -> Vector<T, N>
		{
			Vector<T, N> ret;
			ret[0] = -1;
			return ret;
		}


		template<class T, std::size_t N>
			requires (N > 1)
		constexpr auto Vector<T, N>::Right() noexcept -> Vector<T, N>
		{
			Vector<T, N> ret;
			ret[0] = 1;
			return ret;
		}


		template<class T, std::size_t N>
			requires (N > 1)
		constexpr auto Vector<T, N>::Forward() noexcept -> Vector<T, N>
			requires (N >= 3)
		{
			Vector<T, N> ret;
			ret[2] = 1;
			return ret;
		}


		template<class T, std::size_t N>
			requires (N > 1)
		constexpr auto Vector<T, N>::Backward() noexcept -> Vector<T, N>
			requires (N >= 3)
		{
			Vector<T, N> ret;
			ret[2] = -1;
			return ret;
		}


		template<class T, std::size_t N>
			requires (N > 1)
		constexpr auto Vector<T, N>::Data() const noexcept -> auto&
		{
			return m_Data;
		}


		template<class T, std::size_t N>
			requires (N > 1)
		constexpr auto Vector<T, N>::operator[](size_t index) const noexcept -> auto&
		{
			return GetElementCommon(this, index);
		}


		template<class T, std::size_t N>
			requires (N > 1)
		constexpr auto Vector<T, N>::operator[](size_t const index) noexcept -> auto&
		{
			return GetElementCommon(this, index);
		}


		template<class T, std::size_t N>
			requires (N > 1)
		auto Vector<T, N>::Length() const noexcept -> float
		{
			return math::Sqrt(
			                  static_cast<float>(
				                  std::accumulate(m_Data.begin(), m_Data.end(), static_cast<T>(0), [](T const& sum, T const& elem)
				                  {
					                  return sum + math::Pow(elem, 2);
				                  })));
		}


		template<class T, std::size_t N>
			requires (N > 1)
		auto Vector<T, N>::Normalized() const noexcept -> Vector<T, N>
		{
			return Vector<T, N>{*this}.Normalize();
		}


		template<class T, std::size_t N>
			requires (N > 1)
		auto Vector<T, N>::Normalize() noexcept -> Vector<T, N>&
		{
			if (auto length = Length(); std::abs(length) >= std::numeric_limits<float>::epsilon())
			{
				std::for_each(m_Data.begin(), m_Data.end(), [length](T& elem)
				{
					elem /= static_cast<T>(length);
				});
			}
			return *this;
		}


		template<class T, std::size_t N>
			requires (N > 1)
		constexpr auto Vector<T, N>::Dot(Vector<T, N> const& left, Vector<T, N> const& right) noexcept -> T
		{
			T ret{};
			for (size_t i = 0; i < N; i++)
			{
				ret += left[i] * right[i];
			}
			return ret;
		}


		template<class T, std::size_t N>
			requires (N > 1)
		constexpr auto Vector<T, N>::Cross(Vector<T, N> const& left, Vector<T, N> const& right) noexcept -> Vector<T, N>
			requires (N == 3)
		{
			return Vector<T, N>
			{
				left[1] * right[2] - left[2] * right[1],
				left[2] * right[0] - left[0] * right[2],
				left[0] * right[1] - left[1] * right[0]
			};
		}


		template<class T, std::size_t N>
			requires (N > 1)
		auto Vector<T, N>::Distance(Vector<T, N> const& left, Vector<T, N> const& right) noexcept -> float
		{
			auto sum{0.f};
			for (size_t i = 0; i < N; i++)
			{
				sum += math::Pow(static_cast<float>(left[i] - right[i]), 2);
			}
			return math::Sqrt(sum);
		}


		template<class T, std::size_t N>
			requires (N > 1)
		constexpr auto Vector<T, N>::GetElementCommon(auto* const self, std::size_t const index) -> decltype(auto)
		{
			return self->m_Data[index];
		}


		// Non-member operators

		// Returns a Vector that's components are the additives inverses of this Vector's components.
		template<class T, std::size_t N>
		constexpr auto operator-(Vector<T, N> const& operand) noexcept -> Vector<T, N>
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
		constexpr auto operator+(Vector<T, N> const& left, Vector<T, N> const& right) noexcept -> Vector<T, N>
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
		constexpr auto operator+=(Vector<T, N>& left, Vector<T, N> const& right) noexcept -> Vector<T, N>&
		{
			for (std::size_t i = 0; i < N; i++)
			{
				left[i] += right[i];
			}
			return left;
		}


		// Returns the difference of the input Vectors.
		template<class T, std::size_t N>
		constexpr auto operator-(Vector<T, N> const& left, Vector<T, N> const& right) noexcept -> Vector<T, N>
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
		constexpr auto operator-=(Vector<T, N>& left, Vector<T, N> const& right) noexcept -> Vector<T, N>&
		{
			for (std::size_t i = 0; i < N; i++)
			{
				left[i] -= right[i];
			}
			return left;
		}


		// Returns the result of the scalar multiplication of the input values.
		template<class T1, std::convertible_to<T1> T2, std::size_t N>
		constexpr auto operator*(Vector<T1, N> const& left, T2 const& right) noexcept -> Vector<T1, N>
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
		constexpr auto operator*(T2 const& left, Vector<T1, N> const& right) noexcept -> Vector<T1, N>
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
		constexpr auto operator*(Vector<T, N> const& left, Vector<T, N> const& right) noexcept -> Vector<T, N>
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
		constexpr auto operator*=(Vector<T1, N>& left, T2 const& right) noexcept -> Vector<T1, N>&
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
		constexpr auto operator*=(Vector<T, N>& left, Vector<T, N> const& right) noexcept -> Vector<T, N>&
		{
			for (std::size_t i = 0; i < N; i++)
			{
				left[i] *= right[i];
			}
			return left;
		}


		// Returns the result of the scalar division of the input values.
		template<class T1, std::convertible_to<T1> T2, std::size_t N>
		constexpr auto operator/(Vector<T1, N> const& left, T2 const& right) noexcept -> Vector<T1, N>
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
		constexpr auto operator/(T2 const& left, Vector<T1, N> const& right) noexcept -> Vector<T1, N>
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
		constexpr auto operator/(Vector<T, N> const& left, Vector<T, N> const& right) noexcept -> Vector<T, N>
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
		constexpr auto operator/=(Vector<T1, N>& left, T2 const& right) noexcept -> Vector<T1, N>&
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
		constexpr auto operator/=(Vector<T, N>& left, Vector<T, N> const& right) noexcept -> Vector<T, N>&
		{
			for (std::size_t i = 0; i < N; i++)
			{
				left[i] /= right[i];
			}
			return left;
		}


		// Returns whether the input Vectors are equal.
		template<class T, std::size_t N>
		constexpr auto operator==(Vector<T, N> const& left, Vector<T, N> const& right) noexcept -> bool
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
		constexpr auto operator!=(Vector<T, N> const& left, Vector<T, N> const& right) noexcept -> bool
		{
			return !(left == right);
		}


		// Prints the input Vector on the specified output stream.
		template<class T, std::size_t N>
		auto operator<<(std::ostream& stream, Vector<T, N> const& vector) noexcept -> std::ostream&
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


	// Free functions for Vector
	namespace math
	{
		template<class T, std::size_t N>
		auto Lerp(internal::Vector<T, N> const& from, internal::Vector<T, N> const& to, float const t) -> internal::Vector<T, N>
		{
			return (1 - t) * from + t * to;
		}
	}


	// 4D single-precision floating-point Vector.
	using Vector4 = internal::Vector<float, 4>;
	// 3D single-precision floating-point Vector.
	using Vector3 = internal::Vector<float, 3>;
	// 2D single-precision floating-point Vector.
	using Vector2 = internal::Vector<float, 2>;
}
