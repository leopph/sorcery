#include "Deserialize.hpp"


namespace leopph::convert
{
	auto DeserializeI32(std::span<u8 const> const bytes, std::endian const endianness) -> i32
	{
		auto it = bytes.begin();
		return Deserialize<i32>(it, endianness);
	}


	auto DeserializeU32(std::span<u8 const> const bytes, std::endian const endianness) -> u32
	{
		auto it = bytes.begin();
		return Deserialize<u32>(it, endianness);
	}


	auto DeserializeF32(std::span<u8 const> const bytes, std::endian const endianness) -> f32
	{
		auto it = bytes.begin();
		return Deserialize<f32>(it, endianness);
	}


	auto DeserializeU64(std::span<u8 const> const bytes, std::endian const endianness) -> u64
	{
		auto it = bytes.begin();
		return Deserialize<u64>(it, endianness);
	}
}
