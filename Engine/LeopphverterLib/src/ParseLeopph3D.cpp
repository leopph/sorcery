#include "ParseLeopph3D.hpp"

#include "Compress.hpp"
#include "Deserialize.hpp"
#include "Image.hpp"
#include "Logger.hpp"

#include <bit>
#include <fstream>
#include <memory>
#include <vector>


namespace leopph::convert
{
	auto ParseLeopph3D(std::filesystem::path const& path) -> std::optional<Object>
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

			std::vector<u8> buffer(13);

			// read header
			in.read(reinterpret_cast<char*>(buffer.data()), 13);

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
			auto const contentSize = DeserializeU64({std::begin(buffer) + 5, std::end(buffer)}, endianness);

			buffer.clear();

			// stream rest of the file in
			std::copy(std::istream_iterator<u8>{in}, std::istream_iterator<u8>{}, std::back_inserter(buffer));

			// uncompress data
			std::vector<u8> uncompressed;

			if (compress::Uncompress({std::begin(buffer), std::end(buffer)}, contentSize, uncompressed) != compress::Error::None)
			{
				internal::Logger::Instance().Error("Couldn't parse leopph3d file at " + path.string() + " because the contents failed to uncompress.");
				return {};
			}

			auto it = std::cbegin(uncompressed);

			// number of images
			auto const numImgs = DeserializeU64({it, 8}, endianness);
			it += 8;

			// all images
			std::vector<Image> imgs;
			imgs.reserve(numImgs);

			// parse image data
			for (u64 i = 0; i < numImgs; i++)
			{
				imgs.push_back(DeserializeImage(it, endianness));
			}

			// number of materials
			auto const numMats = DeserializeU64({it, 8}, endianness);
			it += 8;

			// all materials
			std::vector<Material> mats;
			mats.reserve(numMats);

			// parse material data
			for (u64 i = 0; i < numMats; i++)
			{
				mats.push_back(DeserializeMaterial(it, endianness));
			}

			// number of meshes
			auto const numMeshes = DeserializeU64({it, 8}, endianness);
			it += 8;

			// all meshes
			std::vector<Mesh> meshes;
			meshes.reserve(numMeshes);

			// parse mesh data
			for (u64 i = 0; i < numMeshes; i++)
			{
				meshes.push_back(DeserializeMesh(it, endianness));
			}

			return Object
			{
				.Textures = std::move(imgs),
				.Materials = std::move(mats),
				.Meshes = std::move(meshes)
			};
		}
		catch (...)
		{
			internal::Logger::Instance().Error("Couldn't parse leopph3d file at " + path.string() + ". The file may be corrupt.");
			return {};
		}
	}
}
