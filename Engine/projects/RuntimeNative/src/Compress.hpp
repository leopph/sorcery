#pragma once

#include "Core.hpp"

#include <span>
#include <vector>


namespace leopph {
	enum class Error {
		None, Inconsistency, Unknown
	};


	auto Compress(std::span<u8> in, std::vector<u8>& out) -> Error;
	auto Uncompress(std::span<u8> in, u64 uncompressedSize, std::vector<u8>& out) -> Error;
}