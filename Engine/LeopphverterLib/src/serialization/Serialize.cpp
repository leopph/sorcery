#include "Serialize.hpp"

#include <iterator>


namespace leopph::convert
{
	void serialize(std::string_view const str, std::vector<u8>& oBuf, std::endian const endianness)
	{
		auto const sz = str.size();
		serialize(sz, oBuf, endianness);

		std::copy_n(reinterpret_cast<u8 const*>(str.data()), sz, std::back_inserter(oBuf));
	}



	void serialize(Image const& img, std::vector<u8>& oBuf, std::endian const endianness)
	{
		auto const width = img.Width();
		serialize(width, oBuf, endianness);

		auto const height = img.Height();
		serialize(height, oBuf, endianness);

		auto const chans = img.Channels();
		serialize(chans, oBuf);

		auto const encoding = img.Encoding();
		serialize(encoding, oBuf);

		std::copy_n(img.Data().data(), width * height * chans, std::back_inserter(oBuf));
	}



	void serialize(Color const& color, std::vector<u8>& oBuf)
	{
		serialize(color.Red, oBuf);
		serialize(color.Green, oBuf);
		serialize(color.Blue, oBuf);
	}



	void serialize(Material const& mat, std::vector<u8>& oBuf, std::endian const endianness)
	{
		serialize(mat.diffuseColor, oBuf);
		serialize(mat.specularColor, oBuf);
		serialize(mat.gloss, oBuf, endianness);
		serialize(mat.opacity, oBuf, endianness);

		static_assert(sizeof(bool) == 1); // temporary check, find better solution
		serialize(static_cast<u8>(mat.twoSided), oBuf);

		u8 flags{0};
		std::vector<u64> textures;

		if (mat.diffuseMap.has_value())
		{
			flags |= 0x80;
			textures.push_back(mat.diffuseMap.value());
		}

		if (mat.specularMap.has_value())
		{
			flags |= 0x40;
			textures.push_back(mat.specularMap.value());
		}

		if (mat.opacityMap.has_value())
		{
			flags |= 0x20;
			textures.push_back(mat.opacityMap.value());
		}

		serialize(flags, oBuf);

		for (auto const tex : textures)
		{
			serialize(tex, oBuf, endianness);
		}
	}



	void serialize(Vertex const& vert, std::vector<u8>& oBuf, std::endian const endianness)
	{
		serialize(vert.Position, oBuf, endianness);
		serialize(vert.Normal, oBuf, endianness);
		serialize(vert.TexCoord, oBuf, endianness);
	}



	void serialize(Mesh const& mesh, std::vector<u8>& oBuf, std::endian const endianness)
	{
		serialize(mesh.vertices.size(), oBuf, endianness);

		for (auto const& vert : mesh.vertices)
		{
			serialize(vert, oBuf, endianness);
		}

		serialize(mesh.indices.size(), oBuf, endianness);

		for (auto const ind : mesh.indices)
		{
			serialize(ind, oBuf, endianness);
		}

		serialize(mesh.material, oBuf, endianness);
	}
}
