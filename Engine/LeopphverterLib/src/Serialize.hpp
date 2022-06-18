#pragma once

#include "Image.hpp"
#include "LeopphverterTypes.hpp"

#include <bit>
#include <string_view>
#include <type_traits>
#include <vector>


namespace leopph::convert
{
	template<class T>
	concept Scalar = std::is_scalar_v<T>;

	template<Scalar T>
		requires(sizeof(T) == 1)
	auto Serialize(T s, std::vector<u8>& oBuf) -> void;

	template<Scalar T>
		requires(sizeof(T) > 1)
	auto Serialize(T s, std::vector<u8>& oBuf, std::endian endianness) -> void;

	auto Serialize(std::string_view str, std::vector<u8>& oBuf, std::endian endianness) -> void;

	auto Serialize(Image const& img, std::vector<u8>& oBuf, std::endian endianness) -> void;

	auto Serialize(Color const& color, std::vector<u8>& oBuf, std::endian endianness) -> void;

	auto Serialize(Material const& mat, std::vector<u8>& oBuf, std::endian endianness) -> void;

	template<class T, u64 N>
	auto Serialize(internal::Vector<T, N> const& vec, std::vector<u8>& oBuf, std::endian endianness) -> void;

	auto Serialize(Vertex const& vert, std::vector<u8>& oBuf, std::endian endianness) -> void;

	auto Serialize(Mesh const& mesh, std::vector<u8>& oBuf, std::endian endianness) -> void;


	template<Scalar T>
		requires(sizeof(T) == 1)
	auto Serialize(T const s, std::vector<u8>& oBuf) -> void
	{
		oBuf.push_back(*reinterpret_cast<u8 const*>(&s));
	}


	template<Scalar T>
		requires(sizeof(T) > 1)
	auto Serialize(T const s, std::vector<u8>& oBuf, std::endian const endianness) -> void
	{
		auto const* const begin = reinterpret_cast<u8 const*>(&s);
		auto const sz = sizeof(T);
		auto const inserter = std::back_inserter(oBuf);

		if (endianness == std::endian::native)
		{
			std::copy_n(begin, sz, inserter);
			return;
		}

		std::copy_n(std::reverse_iterator{begin + sz}, sz, inserter);
	}


	template<class T, u64 N>
	auto Serialize(internal::Vector<T, N> const& vec, std::vector<u8>& oBuf, std::endian endianness) -> void
	{
		for (u64 i = 0; i < N; i++)
		{
			if constexpr (sizeof(T) == 1 && Scalar<T>)
			{
				Serialize(vec[i], oBuf);
			}
			else
			{
				Serialize(vec[i], oBuf, endianness);
			}
		}
	}
}
