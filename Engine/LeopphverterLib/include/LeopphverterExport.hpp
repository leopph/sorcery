#pragma once

#include "LeopphApi.hpp"
#include "LeopphverterTypes.hpp"

#include <bit>
#include <vector>


namespace leopph::convert
{
	LEOPPHAPI auto Export(Object const& object, std::endian endianness = std::endian::native) -> std::vector<unsigned char>;
}