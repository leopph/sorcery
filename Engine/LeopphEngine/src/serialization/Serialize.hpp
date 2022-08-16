#pragma once
/*
#include "Image.hpp"
#include "StaticModelData.hpp"
#include "Util.hpp"

#include <bit>
#include <type_traits>
#include <vector>


namespace leopph
{
	template<typename T> requires std::is_scalar_v<T> && (sizeof(T) == 1)
	void serialize(T s, std::vector<u8>& oBuf);

	template<typename T> requires std::is_scalar_v<T> && (sizeof(T) > 1)
	void serialize(T s, std::vector<u8>& oBuf, std::endian endianness);

	void serialize(std::string_view str, std::vector<u8>& oBuf, std::endian endianness);

	void serialize(Image const& img, std::vector<u8>& oBuf, std::endian endianness);

	void serialize(MaterialData const& mat, std::vector<u8>& oBuf, std::endian endianness);

	template<class T, u64 N>
	void serialize(Vector<T, N> const& vec, std::vector<u8>& oBuf, std::endian endianness);

	void serialize(Vertex const& vert, std::vector<u8>& oBuf, std::endian endianness);

	void serialize(StaticMeshData const& mesh, std::vector<u8>& oBuf, std::endian endianness);



	template<typename T> requires std::is_scalar_v<T> && (sizeof(T) == 1)
	void serialize(T const s, std::vector<u8>& oBuf)
	{
		oBuf.push_back(*reinterpret_cast<u8 const*>(&s));
	}



	template<typename T> requires std::is_scalar_v<T> && (sizeof(T) > 1)
	void serialize(T const s, std::vector<u8>& oBuf, std::endian const endianness)
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
	void serialize(Vector<T, N> const& vec, std::vector<u8>& oBuf, std::endian endianness)
	{
		for (u64 i = 0; i < N; i++)
		{
			if constexpr (sizeof(T) == 1 && std::is_scalar_v<T>)
			{
				Serialize(vec[i], oBuf);
			}
			else
			{
				serialize(vec[i], oBuf, endianness);
			}
		}
	}
}
*/