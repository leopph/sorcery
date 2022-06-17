#include "ParseLeopph3D.hpp"

#include "Deserialize.hpp"
#include "Image.hpp"
#include "Logger.hpp"

#include <bit>
#include <cstddef>
#include <fstream>
#include <memory>
#include <vector>


namespace leopph::convert
{
	namespace
	{ }


	auto ParseLeopph3D(std::filesystem::path const& path) -> std::optional<Object>
	{
		std::ifstream in{path, std::ios::in | std::ios::binary};

		// disable whitespace skipping
		in.unsetf(std::ios::skipws);

		if (!in.is_open())
		{
			internal::Logger::Instance().Error("Error while parsing leopph3d: the file does not exist.");
			return {};
		}

		std::vector<std::uint8_t> buffer(5);
		in.read(reinterpret_cast<char*>(buffer.data()), 5);

		if (in.eof() || in.fail())
		{
			internal::Logger::Instance().Error("Error while parsing leopph3d: the file is not in valid leopph3d format.");
			return {};
		}

		auto const endianness = buffer[4] & 0x80 ? std::endian::little : std::endian::big;

		buffer.clear();

		std::copy(std::istream_iterator<std::uint8_t>{in}, std::istream_iterator<std::uint8_t>{}, std::back_inserter(buffer));

		auto const& bytes = buffer;
		auto it = std::begin(bytes);

		auto const numImgs = DeserializeU64(std::span<std::uint8_t const, 8>{it, 8}, endianness);
		it += 8;

		std::vector<Image> imgs;
		imgs.reserve(14);

		for (std::uint64_t i = 0; i < numImgs; i++)
		{
			auto const width = DeserializeI32(std::span<std::uint8_t const, 4>{it, 4}, endianness);
			auto const height = DeserializeI32(std::span<std::uint8_t const, 4>{it + 4, 4}, endianness);
			auto const chans = *(it + 8);
			it += 9;

			auto const imgSz = width * height * chans;
			auto imgData = std::make_unique<unsigned char[]>(imgSz);
			std::copy_n(it, imgSz, imgData.get());
			imgs.emplace_back(width, height, chans, std::move(imgData));

			it += imgSz;
		}

		auto const numMats = DeserializeU64(std::span<std::uint8_t const, 8>{it, 8}, endianness);
		it += 8;

		std::vector<Material> mats;
		mats.reserve(numMats);

		for (std::uint64_t i = 0; i < numMats; i++)
		{
			mats.push_back(DeserializeMaterial(it, endianness));
		}

		auto const numMeshes = DeserializeU64(std::span<std::uint8_t const, 8>{it, 8}, endianness);
		it += 8;

		std::vector<Mesh> meshes;
		meshes.reserve(numMeshes);

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
}
