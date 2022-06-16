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
			auto const chans = img.Channels();
			auto const* const p = reinterpret_cast<uint8_t const*>(img.Data().data());
			SerializeNative(width, oBuf);
			SerializeNative(height, oBuf);
			SerializeNative(chans, oBuf);
			oBuf.insert(std::end(oBuf), p, p + width * height * chans);
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
}
