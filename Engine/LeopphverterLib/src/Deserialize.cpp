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

		if (endianness == std::endian::little)
		{
			return static_cast<i32>(static_cast<u32>(bytes[0]) + (static_cast<u32>(bytes[1]) << 8) + (static_cast<u32>(bytes[2]) << 16) + (static_cast<u32>(bytes[3]) << 24));
		}

		// big endian
		return static_cast<i32>((static_cast<u32>(bytes[0]) << 24) + (static_cast<u32>(bytes[1]) << 16) + (static_cast<u32>(bytes[2]) << 8) + static_cast<u32>(bytes[3]));
	}


	auto DeserializeU32(std::span<u8 const> const bytes, std::endian const endianness) -> u32
	{
		if (endianness == std::endian::native)
		{
			return DeserializeNative<u32>(bytes)[0];
		}

		if (endianness == std::endian::little)
		{
			return static_cast<u32>(bytes[0]) + (static_cast<u32>(bytes[1]) << 8) + (static_cast<u32>(bytes[2]) << 16) + (static_cast<u32>(bytes[3]) << 24);
		}

		// big endian
		return (static_cast<u32>(bytes[0]) << 24) + (static_cast<u32>(bytes[1]) << 16) + (static_cast<u32>(bytes[2]) << 8) + static_cast<u32>(bytes[3]);
	}


	auto DeserializeF32(std::span<u8 const> const bytes, std::endian const endianness) -> f32
	{
		if (endianness == std::endian::native)
		{
			return DeserializeNative<f32>(bytes)[0];
		}

		u8 const fBytes[4]{bytes[3], bytes[2], bytes[1], bytes[0]};
		return *reinterpret_cast<f32 const*>(fBytes);
	}


	auto DeserializeU64(std::span<u8 const> const bytes, std::endian const endianness) -> u64
	{
		if (endianness == std::endian::native)
		{
			return DeserializeNative<u64>(bytes)[0];
		}

		if (endianness == std::endian::little)
		{
			return static_cast<u64>(bytes[0]) + (static_cast<u64>(bytes[1]) << 8) + (static_cast<u64>(bytes[2]) << 16) + (static_cast<u64>(bytes[3]) << 24) + (static_cast<u64>(bytes[4]) << 32) + (static_cast<u64>(bytes[5]) << 40) + (static_cast<u64>(bytes[6]) << 48) + (static_cast<u64>(bytes[7]) << 56);
		}

		// big endian
		return (static_cast<u64>(bytes[0]) << 56) + (static_cast<u64>(bytes[1]) << 48) + (static_cast<u64>(bytes[2]) << 40) + (static_cast<u64>(bytes[3]) << 32) + (static_cast<u64>(bytes[4]) << 24) + (static_cast<u64>(bytes[5]) << 16) + (static_cast<u64>(bytes[6]) << 8) + static_cast<u64>(bytes[7]);
	}
}
