#include "Deserialize.hpp"

#include "Logger.hpp"

#include <cstdint>
#include <span>

static_assert(sizeof(float) == 4); // temporary check, find better solution

namespace leopph::convert
{
	namespace
	{
		// Can be used to deserialize a series of simple objects in a native fashion
		template<class T>
		auto DeserializeNative(std::span<std::uint8_t const> const bytes) -> std::span<T const>
		{
			return {reinterpret_cast<T const*>(bytes.data()), bytes.size() * sizeof(std::uint8_t) / sizeof(T)};
		}
	}


	auto DeserializeI32(std::span<std::uint8_t const> const bytes, std::endian const endianness) -> std::int32_t
	{
		if (endianness == std::endian::native)
		{
			return DeserializeNative<std::int32_t>(bytes)[0];
		}
	}


	auto DeserializeU32(std::span<std::uint8_t const> const bytes, std::endian const endianness) -> std::uint32_t
	{
		if (endianness == std::endian::native)
		{
			return DeserializeNative<std::uint32_t>(bytes)[0];
		}
	}


	auto DeserializeU64(std::span<std::uint8_t const> const bytes, std::endian const endianness) -> std::uint64_t
	{
		if (endianness == std::endian::native)
		{
			return DeserializeNative<std::uint64_t>(bytes)[0];
		}
	}


	auto DeserializeF32(std::span<std::uint8_t const> const bytes, std::endian const endianness) -> float
	{
		if (endianness == std::endian::native)
		{
			return DeserializeNative<float>(bytes)[0];
		}
	}
}
