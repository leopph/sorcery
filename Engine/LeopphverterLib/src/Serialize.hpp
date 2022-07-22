#pragma once

#include "Image.hpp"
#include "LeopphverterTypes.hpp"
#include "Util.hpp"

#include <bit>
#include <string_view>
#include <vector>


namespace leopph::convert
{
	template<Scalar T>
		requires(sizeof(T) == 1)
	void Serialize(T s, std::vector<u8>& oBuf);

	template<Scalar T>
		requires(sizeof(T) > 1)
	void Serialize(T s, std::vector<u8>& oBuf, std::endian endianness);

	void Serialize(std::string_view str, std::vector<u8>& oBuf, std::endian endianness);

	void Serialize(Image const& img, std::vector<u8>& oBuf, std::endian endianness);

	void Serialize(Material const& mat, std::vector<u8>& oBuf, std::endian endianness);

	template<class T, u64 N>
	void Serialize(internal::Vector<T, N> const& vec, std::vector<u8>& oBuf, std::endian endianness);

	void Serialize(Vertex const& vert, std::vector<u8>& oBuf, std::endian endianness);

	void Serialize(Mesh const& mesh, std::vector<u8>& oBuf, std::endian endianness);



	template<Scalar T>
		requires(sizeof(T) == 1)
	void Serialize(T const s, std::vector<u8>& oBuf)
	{
		oBuf.push_back(*reinterpret_cast<u8 const*>(&s));
	}



	template<Scalar T>
		requires(sizeof(T) > 1)
	void Serialize(T const s, std::vector<u8>& oBuf, std::endian const endianness)
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
	void Serialize(internal::Vector<T, N> const& vec, std::vector<u8>& oBuf, std::endian endianness)
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
