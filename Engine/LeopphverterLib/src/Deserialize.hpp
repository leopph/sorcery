#pragma once

#include <bit>
#include <cstdint>
#include <span>


namespace leopph::convert
{
	auto DeserializeI32(std::span<std::uint8_t const, 4> bytes, std::endian endianness) -> std::int32_t;

	auto DeserializeU32(std::span<std::uint8_t const, 4> bytes, std::endian endianness) -> std::uint32_t;

	auto DeserializeU64(std::span<std::uint8_t const, 8> bytes, std::endian endianness) -> std::uint64_t;

	auto DeserializeF32(std::span<std::uint8_t const, 4> bytes, std::endian endianness) -> float;
}
