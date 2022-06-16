#pragma once

#include "Image.hpp"

#include <bit>
#include <cstdint>
#include <string_view>
#include <vector>


namespace leopph::convert
{
	auto Serialize(std::int8_t i, std::vector<uint8_t>& oBuf, std::endian endianness) -> void;
	auto Serialize(std::uint8_t u, std::vector<uint8_t>& oBuf, std::endian endianness) -> void;
		 
	auto Serialize(std::int16_t i, std::vector<uint8_t>& oBuf, std::endian endianness) -> void;
	auto Serialize(std::uint16_t u, std::vector<uint8_t>& oBuf, std::endian endianness) -> void;
		 
	auto Serialize(std::int32_t i, std::vector<uint8_t>& oBuf, std::endian endianness) -> void;
	auto Serialize(std::uint32_t u, std::vector<uint8_t>& oBuf, std::endian endianness) -> void;
		 
	auto Serialize(std::int64_t i, std::vector<uint8_t>& oBuf, std::endian endianness) -> void;
	auto Serialize(std::uint64_t u, std::vector<uint8_t>& oBuf, std::endian endianness) -> void;
		 
	auto Serialize(std::string_view str, std::vector<uint8_t>& oBuf, std::endian endianness) -> void;
		 
	auto Serialize(Image const& img, std::vector<uint8_t>& oBuf, std::endian endianness) -> void;


}
