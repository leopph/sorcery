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
		auto const width = static_cast<i32>(img.Width());
		Serialize(width, oBuf, endianness);

		auto const height = static_cast<i32>(img.Height());
		Serialize(height, oBuf, endianness);

		auto const chans = static_cast<u8>(img.Channels());
		Serialize(chans, oBuf);

		std::copy_n(img.Data().data(), width * height * chans, std::back_inserter(oBuf));
	}


	auto Serialize(Color const& color, std::vector<u8>& oBuf, std::endian const endianness) -> void
	{
		Serialize(color.Red, oBuf);
		Serialize(color.Green, oBuf);
		Serialize(color.Blue, oBuf);
	}


	auto Serialize(Material const& mat, std::vector<u8>& oBuf, std::endian const endianness) -> void
	{
		Serialize(mat.DiffuseColor, oBuf, endianness);
		Serialize(mat.SpecularColor, oBuf, endianness);
		Serialize(mat.Gloss, oBuf, endianness);
		Serialize(mat.Opacity, oBuf, endianness);

		static_assert(sizeof(bool) == 1); // temporary check, find better solution
		Serialize(static_cast<u8>(mat.TwoSided), oBuf);

		u8 flags{0};

		if (mat.DiffuseMap.has_value())
		{
			flags |= 0x80;
		}

		if (mat.SpecularMap.has_value())
		{
			flags |= 0x40;
		}

		if (mat.OpacityMap.has_value())
		{
			flags |= 0x20;
		}

		Serialize(flags, oBuf);

		Serialize(mat.DiffuseMap.value_or(0), oBuf, endianness);
		Serialize(mat.SpecularMap.value_or(0), oBuf, endianness);
		Serialize(mat.OpacityMap.value_or(0), oBuf, endianness);
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
