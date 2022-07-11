#include "Serialize.hpp"

#include <iterator>


namespace leopph::convert
{
	auto Serialize(std::string_view const str, std::vector<u8>& oBuf, std::endian const endianness) -> void
	{
		auto const sz = static_cast<u64>(str.size());
		Serialize(sz, oBuf, endianness);

		std::copy_n(reinterpret_cast<u8 const*>(str.data()), sz, std::back_inserter(oBuf));
	}



	auto Serialize(Image const& img, std::vector<u8>& oBuf, std::endian const endianness) -> void
	{
		auto const width = img.Width();
		Serialize(width, oBuf, endianness);

		auto const height = img.Height();
		Serialize(height, oBuf, endianness);

		auto const chans = img.Channels();
		Serialize(chans, oBuf);

		auto const encoding = img.Encoding();
		Serialize(encoding, oBuf);

		std::copy_n(img.Data().data(), width * height * chans, std::back_inserter(oBuf));
	}



	auto Serialize(Color const& color, std::vector<u8>& oBuf) -> void
	{
		Serialize(color.Red, oBuf);
		Serialize(color.Green, oBuf);
		Serialize(color.Blue, oBuf);
	}



	auto Serialize(Material const& mat, std::vector<u8>& oBuf, std::endian const endianness) -> void
	{
		Serialize(mat.DiffuseColor, oBuf);
		Serialize(mat.SpecularColor, oBuf);
		Serialize(mat.Gloss, oBuf, endianness);
		Serialize(mat.Opacity, oBuf, endianness);

		static_assert(sizeof(bool) == 1); // temporary check, find better solution
		Serialize(static_cast<u8>(mat.TwoSided), oBuf);

		u8 flags{0};
		std::vector<u64> textures;

		if (mat.DiffuseMap.has_value())
		{
			flags |= 0x80;
			textures.push_back(mat.DiffuseMap.value());
		}

		if (mat.SpecularMap.has_value())
		{
			flags |= 0x40;
			textures.push_back(mat.SpecularMap.value());
		}

		if (mat.OpacityMap.has_value())
		{
			flags |= 0x20;
			textures.push_back(mat.OpacityMap.value());
		}

		Serialize(flags, oBuf);

		for (auto const tex : textures)
		{
			Serialize(tex, oBuf, endianness);
		}
	}



	auto Serialize(Vertex const& vert, std::vector<u8>& oBuf, std::endian const endianness) -> void
	{
		Serialize(vert.Position, oBuf, endianness);
		Serialize(vert.Normal, oBuf, endianness);
		Serialize(vert.TexCoord, oBuf, endianness);
	}



	auto Serialize(Mesh const& mesh, std::vector<u8>& oBuf, std::endian const endianness) -> void
	{
		Serialize(static_cast<u64>(mesh.Vertices.size()), oBuf, endianness);

		for (auto const& vert : mesh.Vertices)
		{
			Serialize(vert, oBuf, endianness);
		}

		Serialize(static_cast<u64>(mesh.Indices.size()), oBuf, endianness);

		for (auto const ind : mesh.Indices)
		{
			Serialize(ind, oBuf, endianness);
		}

		Serialize(static_cast<u64>(mesh.Material), oBuf, endianness);
	}
}
