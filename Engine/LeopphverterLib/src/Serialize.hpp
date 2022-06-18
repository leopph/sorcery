#pragma once

#include "Image.hpp"
#include "LeopphverterTypes.hpp"

#include <bit>
#include <string_view>
#include <vector>


namespace leopph::convert
{
	auto Serialize(i16 i, std::vector<u8>& oBuf, std::endian endianness) -> void;
	auto Serialize(u16 u, std::vector<u8>& oBuf, std::endian endianness) -> void;

	auto Serialize(i32 i, std::vector<u8>& oBuf, std::endian endianness) -> void;
	auto Serialize(u32 u, std::vector<u8>& oBuf, std::endian endianness) -> void;
	auto Serialize(f32 f, std::vector<u8>& oBuf, std::endian endianness) -> void;

	auto Serialize(i64 i, std::vector<u8>& oBuf, std::endian endianness) -> void;
	auto Serialize(u64 u, std::vector<u8>& oBuf, std::endian endianness) -> void;
	auto Serialize(f64 f, std::vector<u8>& oBuf, std::endian endianness) -> void;

	auto Serialize(std::string_view str, std::vector<u8>& oBuf, std::endian endianness) -> void;

	auto Serialize(Image const& img, std::vector<u8>& oBuf, std::endian endianness) -> void;

	auto Serialize(Color const& color, std::vector<u8>& oBuf, std::endian endianness) -> void;

	auto Serialize(Material const& mat, std::vector<u8>& oBuf, std::endian endianness) -> void;

	auto Serialize(Vector2 const& vec, std::vector<u8>& oBuf, std::endian endianness) -> void;
	auto Serialize(Vector3 const& vec, std::vector<u8>& oBuf, std::endian endianness) -> void;

	auto Serialize(Vertex const& vert, std::vector<u8>& oBuf, std::endian endianness) -> void;

	auto Serialize(Mesh const& mesh, std::vector<u8>& oBuf, std::endian endianness) -> void;
}
