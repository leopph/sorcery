#include "Compress.hpp"
#include "LeopphverterExport.hpp"
#include "Logger.hpp"
#include "Serialize.hpp"

#include <bit>
#include <iterator>
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
		Serialize(object.Textures.size(), toCompress, endianness);

		// write images
		for (auto const& texture : object.Textures)
		{
			// image data
			Serialize(texture, toCompress, endianness);
		}

		// number of materials
		Serialize(object.Materials.size(), toCompress, endianness);

		for (auto const& material : object.Materials)
		{
			// material data
			Serialize(material, toCompress, endianness);
		}

		// number of meshes
		Serialize(object.Meshes.size(), toCompress, endianness);

		for (auto const& mesh : object.Meshes)
		{
			// mesh data
			Serialize(mesh, toCompress, endianness);
		}

		switch (std::vector<u8> compressed; compress::Compress(toCompress, compressed))
		{
			case compress::Error::None:
			{
				// uncompressed data size
				Serialize(toCompress.size(), bytes, endianness);

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
