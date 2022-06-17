#include "Serialization.hpp"


namespace leopph::convert
{
	namespace
	{
		auto SerializeNative(std::int8_t const i, std::vector<uint8_t>& oBuf) -> void
		{
			oBuf.push_back(*reinterpret_cast<std::uint8_t const*>(&i));
		}


		auto SerializeNative(std::uint8_t const u, std::vector<uint8_t>& oBuf) -> void
		{
			oBuf.push_back(u);
		}


		auto SerializeNative(std::int16_t const i, std::vector<uint8_t>& oBuf) -> void
		{
			auto const* const p = reinterpret_cast<uint8_t const*>(&i);
			oBuf.insert(std::end(oBuf), p, p + 2);
		}


		auto SerializeNative(std::uint16_t const u, std::vector<uint8_t>& oBuf) -> void
		{
			auto const* const p = reinterpret_cast<uint8_t const*>(&u);
			oBuf.insert(std::end(oBuf), p, p + 2);
		}


		auto SerializeNative(std::int32_t const i, std::vector<uint8_t>& oBuf) -> void
		{
			auto const* const p = reinterpret_cast<uint8_t const*>(&i);
			oBuf.insert(std::end(oBuf), p, p + 4);
		}


		auto SerializeNative(std::uint32_t const u, std::vector<uint8_t>& oBuf) -> void
		{
			auto const* const p = reinterpret_cast<uint8_t const*>(&u);
			oBuf.insert(std::end(oBuf), p, p + 4);
		}


		auto SerializeNative(std::int64_t const i, std::vector<uint8_t>& oBuf) -> void
		{
			auto const* const p = reinterpret_cast<uint8_t const*>(&i);
			oBuf.insert(std::end(oBuf), p, p + 8);
		}


		auto SerializeNative(std::uint64_t const u, std::vector<uint8_t>& oBuf) -> void
		{
			auto const* const p = reinterpret_cast<uint8_t const*>(&u);
			oBuf.insert(std::end(oBuf), p, p + 8);
		}


		auto SerializeNative(float const f, std::vector<std::uint8_t>& oBuf) -> void
		{
			static_assert(sizeof(float) == 4); // temporary check, find better solution
			auto const* const p = reinterpret_cast<std::uint8_t const*>(&f);
			oBuf.insert(std::end(oBuf), p, p + 4);
		}


		auto SerializeNative(double const d, std::vector<std::uint8_t>& oBuf) -> void
		{
			static_assert(sizeof(double) == 8); // temporary check, find better solution
			auto const* const p = reinterpret_cast<std::uint8_t const*>(&d);
			oBuf.insert(std::end(oBuf), p, p + 8);
		}


		auto SerializeNative(std::string_view const str, std::vector<uint8_t>& oBuf) -> void
		{
			auto const sz = str.size();
			auto const* const p = reinterpret_cast<uint8_t const*>(str.data());
			SerializeNative(sz, oBuf);
			oBuf.insert(std::end(oBuf), p, p + sz);
		}


		auto SerializeNative(Image const& img, std::vector<uint8_t>& oBuf) -> void
		{
			auto const width = img.Width();
			auto const height = img.Height();
			auto const chans = static_cast<uint8_t>(img.Channels());
			auto const* const p = reinterpret_cast<uint8_t const*>(img.Data().data());
			SerializeNative(width, oBuf);
			SerializeNative(height, oBuf);
			SerializeNative(chans, oBuf);
			oBuf.insert(std::end(oBuf), p, p + width * height * chans);
		}


		auto SerializeNative(Color const& color, std::vector<std::uint8_t>& oBuf) -> void
		{
			SerializeNative(color.Red, oBuf);
			SerializeNative(color.Green, oBuf);
			SerializeNative(color.Blue, oBuf);
		}


		auto SerializeNative(Material const& mat, std::vector<std::uint8_t>& oBuf) -> void
		{
			SerializeNative(mat.DiffuseColor, oBuf);
			SerializeNative(mat.SpecularColor, oBuf);
			SerializeNative(mat.Gloss, oBuf);
			SerializeNative(mat.Opacity, oBuf);

			static_assert(sizeof(bool) == 1); // temporary check, find better solution
			SerializeNative(static_cast<std::uint8_t>(mat.TwoSided), oBuf);

			static const std::string texIdPrefix{"tex"}; // double defined, TODO
			if (mat.DiffuseMap.has_value())
			{
				SerializeNative(texIdPrefix + std::to_string(mat.DiffuseMap.value()), oBuf);
			}
			else
			{
				SerializeNative(std::string{}, oBuf);
			}

			if (mat.SpecularMap.has_value())
			{
				SerializeNative(texIdPrefix + std::to_string(mat.SpecularMap.value()), oBuf);
			}
			else
			{
				SerializeNative(std::string{}, oBuf);
			}

			if (mat.OpacityMap.has_value())
			{
				SerializeNative(texIdPrefix + std::to_string(mat.OpacityMap.value()), oBuf);
			}
			else
			{
				SerializeNative(std::string{}, oBuf);
			}
		}
	}


	auto Serialize(std::int8_t const i, std::vector<uint8_t>& oBuf, std::endian const endianness) -> void
	{
		if (endianness == std::endian::native)
		{
			SerializeNative(i, oBuf);
		}
	}


	auto Serialize(std::uint8_t const u, std::vector<uint8_t>& oBuf, std::endian const endianness) -> void
	{
		if (endianness == std::endian::native)
		{
			SerializeNative(u, oBuf);
		}
	}


	auto Serialize(std::int16_t const i, std::vector<uint8_t>& oBuf, std::endian const endianness) -> void
	{
		if (endianness == std::endian::native)
		{
			SerializeNative(i, oBuf);
		}
	}


	auto Serialize(std::uint16_t const u, std::vector<uint8_t>& oBuf, std::endian const endianness) -> void
	{
		if (endianness == std::endian::native)
		{
			SerializeNative(u, oBuf);
		}
	}


	auto Serialize(std::int32_t const i, std::vector<uint8_t>& oBuf, std::endian const endianness) -> void
	{
		if (endianness == std::endian::native)
		{
			SerializeNative(i, oBuf);
		}
	}


	auto Serialize(std::uint32_t const u, std::vector<uint8_t>& oBuf, std::endian const endianness) -> void
	{
		if (endianness == std::endian::native)
		{
			SerializeNative(u, oBuf);
		}
	}


	auto Serialize(std::int64_t const i, std::vector<uint8_t>& oBuf, std::endian const endianness) -> void
	{
		if (endianness == std::endian::native)
		{
			SerializeNative(i, oBuf);
		}
	}


	auto Serialize(std::uint64_t const u, std::vector<uint8_t>& oBuf, std::endian const endianness) -> void
	{
		if (endianness == std::endian::native)
		{
			SerializeNative(u, oBuf);
		}
	}


	auto Serialize(float const f, std::vector<uint8_t>& oBuf, std::endian const endianness) -> void
	{
		if (endianness == std::endian::native)
		{
			SerializeNative(f, oBuf);
		}
	}


	auto Serialize(double const d, std::vector<uint8_t>& oBuf, std::endian const endianness) -> void
	{
		if (endianness == std::endian::native)
		{
			SerializeNative(d, oBuf);
		}
	}


	auto Serialize(std::string_view const str, std::vector<uint8_t>& oBuf, std::endian const endianness) -> void
	{
		if (endianness == std::endian::native)
		{
			SerializeNative(str, oBuf);
		}
	}


	auto Serialize(Image const& img, std::vector<uint8_t>& oBuf, std::endian const endianness) -> void
	{
		if (endianness == std::endian::native)
		{
			SerializeNative(img, oBuf);
		}
	}


	auto Serialize(Color const& color, std::vector<uint8_t>& oBuf, std::endian const endianness) -> void
	{
		if (endianness == std::endian::native)
		{
			SerializeNative(color, oBuf);
		}
	}


	auto Serialize(Material const& mat, std::vector<uint8_t>& oBuf, std::endian const endianness) -> void
	{
		if (endianness == std::endian::native)
		{
			SerializeNative(mat, oBuf);
		}
	}
}
