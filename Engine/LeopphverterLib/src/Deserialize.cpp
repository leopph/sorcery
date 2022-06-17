#include "Deserialize.hpp"

#include "Logger.hpp"

#include <cstdint>
#include <span>


namespace leopph::convert
{
	namespace
	{
		template<class T>
		auto DeserializeNative(std::span<std::uint8_t const> const bytes) -> std::span<T const>
		{
			#ifdef _DEBUG
			if (bytes.size() * sizeof(std::uint8_t) % sizeof(T) != 0)
			{
				internal::Logger::Instance().Error(std::string{"Error: trying to deserialize object(s) of type ["} + typeid(T).name() + "] from an inappropriate number of bytes.");
				return {};
			}
			#else
			#endif

			return {reinterpret_cast<T const*>(bytes.data()), bytes.size() * sizeof(std::uint8_t) / sizeof(T)};
		}
	}


	auto DeserializeI32(std::span<std::uint8_t const, 4> const bytes, std::endian const endianness) -> std::int32_t
	{
		if (endianness == std::endian::native)
		{
			return DeserializeNative<std::int32_t>(bytes)[0];
		}
	}


	auto DeserializeU32(std::span<std::uint8_t const, 4> const bytes, std::endian const endianness) -> std::uint32_t
	{
		if (endianness == std::endian::native)
		{
			return DeserializeNative<std::uint32_t>(bytes)[0];
		}
	}


	auto DeserializeU64(std::span<std::uint8_t const, 8> const bytes, std::endian const endianness) -> std::uint64_t
	{
		if (endianness == std::endian::native)
		{
			return DeserializeNative<std::uint64_t>(bytes)[0];
		}
	}


	auto DeserializeF32(std::span<std::uint8_t const, 4> const bytes, std::endian const endianness) -> float
	{
		if (endianness == std::endian::native)
		{
			static_assert(sizeof(float) == 4); // temporary check, find better solution
			return DeserializeNative<float>(bytes)[0];
		}
	}


	auto DeserializeVec2(std::span<std::uint8_t const, sizeof(Vector2)> const bytes, std::endian const endianness) -> Vector2
	{
		if (endianness == std::endian::native)
		{
			auto const elems = DeserializeNative<float>(bytes);
			return Vector2{elems[0], elems[1]};
		}
	}


	auto DeserializeVec3(std::span<std::uint8_t const, sizeof(Vector3)> const bytes, std::endian const endianness) -> Vector3
	{
		if (endianness == std::endian::native)
		{
			auto const elems = DeserializeNative<float>(bytes);
			return Vector3{elems[0], elems[1], elems[2]};
		}
	}


	auto DeserializeVertex(std::span<std::uint8_t const, sizeof(Vertex)> bytes, std::endian const endianness) -> Vertex
	{
		return Vertex
		{
			DeserializeVec3(std::span<std::uint8_t const, sizeof(Vector3)>{std::begin(bytes), sizeof(Vector3)}, endianness),
			DeserializeVec3(std::span<std::uint8_t const, sizeof(Vector3)>{std::begin(bytes) + sizeof(Vector3), sizeof(Vector3)}, endianness),
			DeserializeVec2(std::span<std::uint8_t const, sizeof(Vector2)>{std::begin(bytes) + 2 * sizeof(Vector3), sizeof(Vector2)}, endianness),
		};
	}
}
