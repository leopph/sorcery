#include "Deserialize.hpp"

#include <cstdint>
#include <span>


namespace leopph::convert
{
	namespace
	{
		// Can be used to deserialize a series of simple objects in a native fashion
		template<class T>
		auto DeserializeNative(std::span<u8 const> const bytes) -> std::span<T const>
		{
			return {reinterpret_cast<T const*>(bytes.data()), bytes.size() * sizeof(u8) / sizeof(T)};
		}
	}


	auto DeserializeI32(std::span<u8 const> const bytes, std::endian const endianness) -> i32
	{
		if (endianness == std::endian::native)
		{
			return DeserializeNative<i32>(bytes)[0];
		}
	}


	auto DeserializeU32(std::span<u8 const> const bytes, std::endian const endianness) -> u32
	{
		if (endianness == std::endian::native)
		{
			return DeserializeNative<u32>(bytes)[0];
		}
	}


	auto DeserializeF32(std::span<u8 const> const bytes, std::endian const endianness) -> f32
	{
		if (endianness == std::endian::native)
		{
			return DeserializeNative<f32>(bytes)[0];
		}
	}


	auto DeserializeU64(std::span<u8 const> const bytes, std::endian const endianness) -> u64
	{
		if (endianness == std::endian::native)
		{
			return DeserializeNative<u64>(bytes)[0];
		}
	}
}
