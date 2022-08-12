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



	std::vector<StaticMeshData> import_static_leopph_3d(std::filesystem::path const& path)
	{
		try
		{
			std::ifstream in{path, std::ios::in | std::ios::binary};

			// disable whitespace skipping
			in.unsetf(std::ios::skipws);

			// failed to open file
			if (!in.is_open())
			{
				internal::Logger::Instance().Error("Can't parse leopph3d file at " + path.string() + " because the file does not exist.");
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
				internal::Logger::Instance().Error("Can't parse leopph3d file at " + path.string() + " because the file is not in valid leopph3d format.");
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
			if (compress::uncompress({std::begin(buffer), std::end(buffer)}, contentSize, uncompressed) != compress::Error::None)
			{
				internal::Logger::Instance().Error("Couldn't parse leopph3d file at " + path.string() + " because the contents failed to uncompress.");
				return {};
			}

			#ifndef NDEBUG
			internal::Logger::Instance().CurrentLevel(internal::Logger::Level::Debug);
			internal::Logger::Instance().Debug(std::format("Parsing leopph3d file at {}.", path.string()));
			#endif

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
			std::vector<MaterialData> meshes;
			meshes.reserve(numMeshes);

			// parse mesh data
			for (u64 i = 0; i < numMeshes; i++)
			{
				meshes.push_back(deserialize_mesh(it, endianness));
			}

			return Object
			{
				.textures = std::move(imgs),
				.materials = std::move(mats),
				.meshes = std::move(meshes)
			};
		}
		catch (...)
		{
			internal::Logger::Instance().Error("Couldn't parse leopph3d file at " + path.string() + ". The file may be corrupt.");
			return {};
		}
	}
}
