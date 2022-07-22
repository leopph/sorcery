#include "Leopphverter.hpp"
#include "Logger.hpp"
#include "../../compression/Compress.hpp"
#include "../../serialization/serialize.hpp"

#include <bit>
#include <iterator>
#include <stdexcept>


namespace leopph::convert
{
	std::vector<unsigned char> encode_in_leopph3d(Object const& object, std::endian const endianness)
	{
		if (endianness != std::endian::little && endianness != std::endian::big)
		{
			auto const msg = "Error while exporting. Mixed endian architectures are currently not supported.";
			internal::Logger::Instance().Critical(msg);
			throw std::logic_error{msg};
		}

		std::vector<u8> bytes;

		// signature bytes
		bytes.push_back('x');
		bytes.push_back('d');
		bytes.push_back('6');
		bytes.push_back('9');

		// the version number. its MSB is used to indicate if the file is little endian
		bytes.push_back(0x01 | (endianness == std::endian::big ? 0 : 0x80));

		std::vector<u8> toCompress;

		// number of images
		serialize(object.textures.size(), toCompress, endianness);

		// write images
		for (auto const& texture : object.textures)
		{
			// image data
			serialize(texture, toCompress, endianness);
		}

		// number of materials
		serialize(object.materials.size(), toCompress, endianness);

		for (auto const& material : object.materials)
		{
			// material data
			serialize(material, toCompress, endianness);
		}

		// number of meshes
		serialize(object.meshes.size(), toCompress, endianness);

		for (auto const& mesh : object.meshes)
		{
			// mesh data
			serialize(mesh, toCompress, endianness);
		}

		switch (std::vector<u8> compressed; compress::compress(toCompress, compressed))
		{
			case compress::Error::None:
			{
				// uncompressed data size
				serialize(toCompress.size(), bytes, endianness);

				// compressed data
				std::ranges::copy(compressed, std::back_inserter(bytes));

				return bytes;
			}

			case compress::Error::Inconsistency:
			{
				internal::Logger::Instance().Error("An inconsistency error occurred while compressing leopph3d contents.");
			}

			case compress::Error::Unknown:
			{
				internal::Logger::Instance().Error("An unknown error occurred while compressing leopph3d contents.");
			}
		}

		return {};
	}
}
