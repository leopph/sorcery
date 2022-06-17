#include "ParseLeopph3D.hpp"

#include "Deserialize.hpp"
#include "Image.hpp"
#include "Logger.hpp"

#include <bit>
#include <fstream>
#include <memory>
#include <vector>


namespace leopph::convert
{
	namespace
	{ }


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

			std::vector<std::uint8_t> buffer(5);

			// read header
			in.read(reinterpret_cast<char*>(buffer.data()), 5);

			// failed to read header
			if (in.eof() || in.fail())
			{
				internal::Logger::Instance().Error("Can't parse leopph3d file at " + path.string() + " because the file is not in valid leopph3d format.");
				return {};
			}

			// parse endianness
			auto const endianness = buffer[4] & 0x80 ? std::endian::little : std::endian::big;

			buffer.clear();

			// stream rest of the file in
			std::copy(std::istream_iterator<std::uint8_t>{in}, std::istream_iterator<std::uint8_t>{}, std::back_inserter(buffer));

			auto const& bytes = buffer;
			auto it = std::begin(bytes);

			// number of images
			auto const numImgs = DeserializeU64({it, 8}, endianness);
			it += 8;

			// all images
			std::vector<Image> imgs;
			imgs.reserve(numImgs);

			// parse image data
			for (std::uint64_t i = 0; i < numImgs; i++)
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
			for (std::uint64_t i = 0; i < numMats; i++)
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
			for (std::uint64_t i = 0; i < numMeshes; i++)
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
