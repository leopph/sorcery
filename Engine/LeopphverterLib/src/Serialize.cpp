#include "Serialize.hpp"

#include <zlib.h>


namespace leopph::convert
{
	namespace
	{
		auto SerializeNative(i16 const i, std::vector<u8>& oBuf) -> void
		{
			auto const* const p = reinterpret_cast<u8 const*>(&i);
			oBuf.insert(std::end(oBuf), p, p + 2);
		}


		auto SerializeNative(u16 const u, std::vector<u8>& oBuf) -> void
		{
			auto const* const p = reinterpret_cast<u8 const*>(&u);
			oBuf.insert(std::end(oBuf), p, p + 2);
		}


		auto SerializeNative(i32 const i, std::vector<u8>& oBuf) -> void
		{
			auto const* const p = reinterpret_cast<u8 const*>(&i);
			oBuf.insert(std::end(oBuf), p, p + 4);
		}


		auto SerializeNative(u32 const u, std::vector<u8>& oBuf) -> void
		{
			auto const* const p = reinterpret_cast<u8 const*>(&u);
			oBuf.insert(std::end(oBuf), p, p + 4);
		}


		auto SerializeNative(f32 const f, std::vector<u8>& oBuf) -> void
		{
			auto const* const p = reinterpret_cast<u8 const*>(&f);
			oBuf.insert(std::end(oBuf), p, p + 4);
		}


		auto SerializeNative(i64 const i, std::vector<u8>& oBuf) -> void
		{
			auto const* const p = reinterpret_cast<u8 const*>(&i);
			oBuf.insert(std::end(oBuf), p, p + 8);
		}


		auto SerializeNative(u64 const u, std::vector<u8>& oBuf) -> void
		{
			auto const* const p = reinterpret_cast<u8 const*>(&u);
			oBuf.insert(std::end(oBuf), p, p + 8);
		}


		auto SerializeNative(f64 const f, std::vector<u8>& oBuf) -> void
		{
			auto const* const p = reinterpret_cast<u8 const*>(&f);
			oBuf.insert(std::end(oBuf), p, p + 8);
		}


		auto SerializeNative(std::string_view const str, std::vector<u8>& oBuf) -> void
		{
			auto const sz = str.size();
			auto const* const p = reinterpret_cast<u8 const*>(str.data());
			SerializeNative(sz, oBuf);
			oBuf.insert(std::end(oBuf), p, p + sz);
		}


		auto SerializeNative(Image const& img, std::vector<u8>& oBuf) -> void
		{
			auto const width = img.Width();
			SerializeNative(width, oBuf);

			auto const height = img.Height();
			SerializeNative(height, oBuf);

			auto const chans = static_cast<u8>(img.Channels());
			oBuf.push_back(chans);
			
			auto comprBufSz = compressBound(width * height * chans);
			auto const comprBuf = std::make_unique_for_overwrite<u8[]>(comprBufSz);
			compress2(comprBuf.get(), &comprBufSz, reinterpret_cast<u8 const*>(img.Data().data()), width * height * chans, 9);
			SerializeNative(static_cast<u64>(comprBufSz), oBuf);
			oBuf.insert(std::end(oBuf), comprBuf.get(), comprBuf.get() + comprBufSz);
		}


		auto SerializeNative(Color const& color, std::vector<u8>& oBuf) -> void
		{
			auto const* const p = reinterpret_cast<u8 const*>(&color);
			oBuf.insert(std::end(oBuf), p, p + 3);
		}


		auto SerializeNative(Material const& mat, std::vector<u8>& oBuf) -> void
		{
			SerializeNative(mat.DiffuseColor, oBuf);
			SerializeNative(mat.SpecularColor, oBuf);
			SerializeNative(mat.Gloss, oBuf);
			SerializeNative(mat.Opacity, oBuf);

			static_assert(sizeof(bool) == 1); // temporary check, find better solution
			oBuf.push_back(static_cast<u8>(mat.TwoSided));

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

			oBuf.push_back(flags);
			SerializeNative(mat.DiffuseMap.value_or(0), oBuf);
			SerializeNative(mat.SpecularMap.value_or(0), oBuf);
			SerializeNative(mat.OpacityMap.value_or(0), oBuf);
		}


		auto SerializeNative(Vector2 const& vec, std::vector<u8>& oBuf) -> void
		{
			SerializeNative(vec[0], oBuf);
			SerializeNative(vec[1], oBuf);
		}


		auto SerializeNative(Vector3 const& vec, std::vector<u8>& oBuf) -> void
		{
			SerializeNative(vec[0], oBuf);
			SerializeNative(vec[1], oBuf);
			SerializeNative(vec[2], oBuf);
		}


		auto SerializeNative(Vertex const& vert, std::vector<u8>& oBuf) -> void
		{
			SerializeNative(vert.Position, oBuf);
			SerializeNative(vert.Normal, oBuf);
			SerializeNative(vert.TexCoord, oBuf);
		}


		auto SerializeNative(Mesh const& mesh, std::vector<u8>& oBuf) -> void
		{
			SerializeNative(mesh.Vertices.size(), oBuf);

			for (auto const& vert : mesh.Vertices)
			{
				SerializeNative(vert, oBuf);
			}

			SerializeNative(mesh.Indices.size(), oBuf);

			for (auto const ind : mesh.Indices)
			{
				SerializeNative(ind, oBuf);
			}

			SerializeNative(mesh.Material, oBuf);
		}
	}


	auto Serialize(i16 const i, std::vector<u8>& oBuf, std::endian const endianness) -> void
	{
		if (endianness == std::endian::native)
		{
			SerializeNative(i, oBuf);
		}
	}


	auto Serialize(u16 const u, std::vector<u8>& oBuf, std::endian const endianness) -> void
	{
		if (endianness == std::endian::native)
		{
			SerializeNative(u, oBuf);
		}
	}


	auto Serialize(i32 const i, std::vector<u8>& oBuf, std::endian const endianness) -> void
	{
		if (endianness == std::endian::native)
		{
			SerializeNative(i, oBuf);
		}
	}


	auto Serialize(u32 const u, std::vector<u8>& oBuf, std::endian const endianness) -> void
	{
		if (endianness == std::endian::native)
		{
			SerializeNative(u, oBuf);
		}
	}


	auto Serialize(f32 const f, std::vector<u8>& oBuf, std::endian const endianness) -> void
	{
		if (endianness == std::endian::native)
		{
			SerializeNative(f, oBuf);
		}
	}


	auto Serialize(i64 const i, std::vector<u8>& oBuf, std::endian const endianness) -> void
	{
		if (endianness == std::endian::native)
		{
			SerializeNative(i, oBuf);
		}
	}


	auto Serialize(u64 const u, std::vector<u8>& oBuf, std::endian const endianness) -> void
	{
		if (endianness == std::endian::native)
		{
			SerializeNative(u, oBuf);
		}
	}


	auto Serialize(f64 const f, std::vector<u8>& oBuf, std::endian const endianness) -> void
	{
		if (endianness == std::endian::native)
		{
			SerializeNative(f, oBuf);
		}
	}


	auto Serialize(std::string_view const str, std::vector<u8>& oBuf, std::endian const endianness) -> void
	{
		if (endianness == std::endian::native)
		{
			SerializeNative(str, oBuf);
		}
	}


	auto Serialize(Image const& img, std::vector<u8>& oBuf, std::endian const endianness) -> void
	{
		if (endianness == std::endian::native)
		{
			SerializeNative(img, oBuf);
		}
	}


	auto Serialize(Color const& color, std::vector<u8>& oBuf, std::endian const endianness) -> void
	{
		if (endianness == std::endian::native)
		{
			SerializeNative(color, oBuf);
		}
	}


	auto Serialize(Material const& mat, std::vector<u8>& oBuf, std::endian const endianness) -> void
	{
		if (endianness == std::endian::native)
		{
			SerializeNative(mat, oBuf);
		}
	}


	auto Serialize(Vector2 const& vec, std::vector<u8>& oBuf, std::endian const endianness) -> void
	{
		if (endianness == std::endian::native)
		{
			SerializeNative(vec, oBuf);
		}
	}


	auto Serialize(Vector3 const& vec, std::vector<u8>& oBuf, std::endian const endianness) -> void
	{
		if (endianness == std::endian::native)
		{
			SerializeNative(vec, oBuf);
		}
	}


	auto Serialize(Vertex const& vert, std::vector<u8>& oBuf, std::endian const endianness) -> void
	{
		if (endianness == std::endian::native)
		{
			SerializeNative(vert, oBuf);
		}
	}


	auto Serialize(Mesh const& mesh, std::vector<u8>& oBuf, std::endian const endianness) -> void
	{
		if (endianness == std::endian::native)
		{
			SerializeNative(mesh, oBuf);
		}
	}
}
