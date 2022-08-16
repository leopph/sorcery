#include "ImportLeopph3d.hpp"

#include "Compress.hpp"
#include "Image.hpp"
#include "Logger.hpp"
#include "../../../serialization/Deserialize.hpp"

#include <bit>
#include <format>
#include <fstream>
#include <memory>
#include <vector>


namespace leopph
{
	namespace
	{
		constexpr u64 HEADER_SZ = 13;
	}



	StaticModelData import_static_leopph_3d(std::filesystem::path const& path)
	{
		/*try
		{
			std::ifstream in{path, std::ios::in | std::ios::binary};

			// disable whitespace skipping
			in.unsetf(std::ios::skipws);

			// failed to open file
			if (!in.is_open())
			{
				Logger::get_instance().error(std::format("Failed to parse leopph3d file at [{}]: the file does not exist.", path.string()));
				return {};
			}

			std::vector<u8> buffer(HEADER_SZ);

			// read header
			in.read(reinterpret_cast<char*>(buffer.data()), HEADER_SZ);

			// failed to read header
			if (in.eof() || in.fail() ||
				buffer[0] != 'x' ||
				buffer[1] != 'd' ||
				buffer[2] != '6' ||
				buffer[3] != '9')
			{
				Logger::get_instance().error(std::format("Failed to parse leopph3d file at [{}]: the file is corrupted or invalid.", path.string()));
				return {};
			}

			// parse endianness
			auto const endianness = buffer[4] & 0x80 ? std::endian::little : std::endian::big;

			// parse content size
			auto const contentSize = deserialize<u64>(std::begin(buffer) + 5, endianness);

			// get the size of the compressed contents
			in.seekg(0, std::ios_base::end);
			auto const comprSz = static_cast<u64>(in.tellg()) - HEADER_SZ;

			// read rest of the file
			buffer.resize(comprSz);
			in.seekg(HEADER_SZ, std::ios_base::beg);
			in.read(reinterpret_cast<char*>(buffer.data()), comprSz);

			std::vector<u8> uncompressed;

			// uncompress data
			if (uncompress({std::begin(buffer), std::end(buffer)}, contentSize, uncompressed) != CompressionError::None)
			{
				Logger::get_instance().error(std::format("Failed to parse leopph3d file at [{}]: the file contents could not be uncompressed.", path.string()));
				return {};
			}
			Logger::get_instance().trace(std::format("Parsing leopph3d file at {}.", path.string()));

			auto it = std::cbegin(uncompressed);

			// number of images
			auto const numImgs = deserialize<u64>(it, endianness);

			// all images
			std::vector<Image> imgs;
			imgs.reserve(numImgs);

			// parse image data
			for (u64 i = 0; i < numImgs; i++)
			{
				imgs.push_back(deserialize_image(it, endianness));
			}

			// number of materials
			auto const numMats = deserialize<u64>(it, endianness);

			// all materials
			std::vector<MaterialData> mats;
			mats.reserve(numMats);

			// parse material data
			for (u64 i = 0; i < numMats; i++)
			{
				mats.push_back(deserialize_material(it, endianness));
			}

			// number of meshes
			auto const numMeshes = deserialize<u64>(it, endianness);

			// all meshes
			std::vector<StaticMeshData> meshes;
			meshes.reserve(numMeshes);

			// parse mesh data
			for (u64 i = 0; i < numMeshes; i++)
			{
				meshes.push_back(deserialize_mesh(it, endianness));
			}

			return StaticModelData
			{
				.meshes = std::move(meshes),
				.materials = std::move(mats),
				.textures = std::move(imgs)
			};
		}
		catch (...)
		{
			internal::Logger::get_instance().error(std::format("Failed to parse leopph3d file at [{}]: an unknown error occured while reading file contents.", path.string()));
			return {};
		}*/

		return {};
	}
}