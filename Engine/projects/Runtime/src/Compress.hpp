#pragma once

#include "LeopphApi.hpp"
#include "Types.hpp"

#include <span>
#include <vector>


namespace leopph
{
	enum class CompressionError
	{
		None, Inconsistency, Unknown
	};


	LEOPPHAPI CompressionError compress(std::span<u8> in, std::vector<u8>& out);
	LEOPPHAPI CompressionError uncompress(std::span<u8> in, u64 uncompressedSize, std::vector<u8>& out);
}
