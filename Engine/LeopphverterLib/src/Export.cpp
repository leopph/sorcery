#include "LeopphverterExport.hpp"
#include "Logger.hpp"
#include "Serialize.hpp"

#include <bit>
#include <cstdint>
#include <stdexcept>


namespace leopph::convert
{
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
		for (auto const& texture : object.Textures)
		{
			// image data
			Serialize(texture, bytes, endianness);
		}

		// number of materials
		Serialize(object.Materials.size(), bytes, endianness);

		for (auto const& material : object.Materials)
		{
			// material data
			Serialize(material, bytes, endianness);
		}

		// number of meshes
		Serialize(object.Meshes.size(), bytes, endianness);

		for (auto const& mesh : object.Meshes)
		{
			// mesh data
			Serialize(mesh, bytes, endianness);
		}

		return bytes;
	}
}
