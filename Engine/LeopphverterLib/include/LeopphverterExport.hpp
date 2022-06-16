#pragma once

#include "LeopphApi.hpp"
#include "LeopphverterCommon.hpp"
//tmp
#include "../src/Serialization.hpp"

#include <bit>
#include <vector>


namespace leopph::convert
{
	LEOPPHAPI auto Export(Object const& object, std::endian endianness = std::endian::native) -> std::vector<unsigned char>;
}
