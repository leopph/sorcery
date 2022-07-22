#pragma once

#include "Types.hpp"

#include <span>
#include <vector>


namespace leopph::convert::compress
{
	enum class Error
	{
		None, Inconsistency, Unknown
	};


	Error Compress(std::span<u8> in, std::vector<u8>& out);
	Error Uncompress(std::span<u8> in, u64 uncompressedSize, std::vector<u8>& out);
}
