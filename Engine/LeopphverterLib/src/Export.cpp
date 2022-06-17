#include "LeopphverterExport.hpp"
#include "Logger.hpp"

#include <bit>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <zlib.h>

// temporary assertions, will be later resolved with platform-based fixed-size types and macros
static_assert(sizeof(char) == 1);
static_assert(sizeof(unsigned char) == 1);
static_assert(sizeof(short) == 2);
static_assert(sizeof(unsigned short) == 2);
static_assert(sizeof(int) == 4);
static_assert(sizeof(unsigned) == 4);
static_assert(sizeof(long) == 4);
static_assert(sizeof(unsigned long) == 4);
static_assert(sizeof(long long) == 8);
static_assert(sizeof(unsigned long long) == 8);
static_assert(std::is_same_v<std::size_t, unsigned long long>);
static_assert(std::endian::native == std::endian::little || std::endian::native == std::endian::big);


namespace leopph::convert
{
	namespace
	{
		constexpr auto SystemBigEndian() -> bool
		{
			return std::endian::native == std::endian::big; // does not take mixed endianness into account
		}
	}


	auto Export(Object const& object, std::endian const endianness) -> std::vector<unsigned char>
	{
		if (endianness != std::endian::little && endianness != std::endian::big)
		{
			auto const msg = "Error while exporting. Mixed endian architectures are currently not supported.";
			internal::Logger::Instance().Critical(msg);
			throw std::logic_error{msg};
		}

		std::vector<unsigned char> bytes;

		// signature bytes
		bytes.push_back('x');
		bytes.push_back('d');
		bytes.push_back('6');
		bytes.push_back('9');

		// the version number. its MSB is used to indicate if the file is little endian
		bytes.push_back(0x01 | (endianness == std::endian::big ? 0 : 0x80));

		// number of images
		Serialize(object.Textures.size(), bytes, endianness);

		// write images
		for (std::size_t i = 0; i < object.Textures.size(); i++)
		{
			std::string const static texIdPrefix{"tex"};
			// id of image
			Serialize(texIdPrefix + std::to_string(i), bytes, endianness);
			// image data
			Serialize(object.Textures[i], bytes, endianness);
		}

		// number of materials
		Serialize(object.Materials.size(), bytes, endianness);

		for (std::size_t i = 0; i < object.Materials.size(); i++)
		{
			std::string const static matIdPrefix{"mat"};
			// id of the material
			Serialize(matIdPrefix + std::to_string(i), bytes, endianness);
			// material data
			Serialize(object.Materials[i], bytes, endianness);
		}


		return bytes;
	}
}
