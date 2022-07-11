#pragma once

#include "LeopphverterTypes.hpp"
#include "Util.hpp"

#include <array>
#include <bit>
#include <iterator>


namespace leopph::convert
{
	template<Scalar T, std::contiguous_iterator It>
		requires(sizeof(T) == 1)
	auto Deserialize(It const& it) -> T;

	template<Scalar T, std::contiguous_iterator It>
		requires(sizeof(T) == 1)
	auto Deserialize(It& it) -> T;

	template<Scalar T, std::contiguous_iterator It>
		requires(sizeof(T) > 1)
	auto Deserialize(It const& it, std::endian endianness) -> T;

	template<Scalar T, std::contiguous_iterator It>
		requires(sizeof(T) > 1)
	auto Deserialize(It& it, std::endian endianness) -> T;

	template<std::contiguous_iterator It>
	auto DeserializeVec2(It& it, std::endian endianness) -> Vector2;

	template<std::contiguous_iterator It>
	auto DeserializeVec3(It& it, std::endian endianness) -> Vector3;

	template<std::contiguous_iterator It>
	auto DeserializeVertex(It& it, std::endian endianness) -> Vertex;

	template<std::contiguous_iterator It>
	auto DeserializeMaterial(It& it, std::endian endianness) -> Material;

	template<std::contiguous_iterator It>
	auto DeserializeMesh(It& it, std::endian endianness) -> Mesh;



	template<Scalar T, std::contiguous_iterator It>
		requires (sizeof(T) == 1)
	auto Deserialize(It const& it) -> T
	{
		return *reinterpret_cast<T const*>(&*it);
	}



	template<Scalar T, std::contiguous_iterator It>
		requires (sizeof(T) == 1)
	auto Deserialize(It& it) -> T
	{
		auto const ret = Deserialize<T>(const_cast<It const&>(it));
		it += 1;
		return ret;
	}



	template<Scalar T, std::contiguous_iterator It>
		requires (sizeof(T) > 1)
	auto Deserialize(It const& it, std::endian const endianness) -> T
	{
		auto static constexpr typeSz = sizeof(T);
		auto const* const p = reinterpret_cast<u8 const*>(&*it);

		if (endianness == std::endian::native)
		{
			return *reinterpret_cast<T const*>(p);
		}

		std::array<u8, typeSz> tmpBytes;
		std::copy_n(std::reverse_iterator{p + typeSz}, typeSz, tmpBytes.data());
		return *reinterpret_cast<T const*>(tmpBytes.data());
	}



	template<Scalar T, std::contiguous_iterator It>
		requires(sizeof(T) > 1)
	auto Deserialize(It& it, std::endian const endianness) -> T
	{
		auto const ret = Deserialize<T>(const_cast<It const&>(it), endianness);
		it += sizeof(T);
		return ret;
	}



	template<std::contiguous_iterator It>
	auto DeserializeVec2(It& it, std::endian endianness) -> Vector2
	{
		return Vector2
		{
			Deserialize<f32>(it, endianness),
			Deserialize<f32>(it, endianness)
		};
	}



	template<std::contiguous_iterator It>
	auto DeserializeVec3(It& it, std::endian endianness) -> Vector3
	{
		return Vector3
		{
			Deserialize<f32>(it, endianness),
			Deserialize<f32>(it, endianness),
			Deserialize<f32>(it, endianness)
		};
	}



	template<std::contiguous_iterator It>
	auto DeserializeVertex(It& it, std::endian const endianness) -> Vertex
	{
		return Vertex
		{
			.Position = DeserializeVec3(it, endianness),
			.Normal = DeserializeVec3(it, endianness),
			.TexCoord = DeserializeVec2(it, endianness)
		};
	}



	template<std::contiguous_iterator It>
	auto DeserializeColor(It& it) -> Color
	{
		return Color
		{
			Deserialize<u8>(it),
			Deserialize<u8>(it),
			Deserialize<u8>(it)
		};
	}



	template<std::contiguous_iterator It>
	auto DeserializeImage(It& it, std::endian const endianness) -> Image
	{
		auto const width = Deserialize<u32>(it, endianness);
		auto const height = Deserialize<u32>(it, endianness);
		auto const chans = Deserialize<u8>(it);
		auto const encoding = Deserialize<u8>(it);

		auto imgSz = static_cast<u64>(width) * height * chans;
		auto imgData = std::make_unique<unsigned char[]>(imgSz);
		std::copy_n(it, width * height * chans, imgData.get());
		it += imgSz;

		return Image{width, height, chans, std::move(imgData), static_cast<ColorEncoding>(encoding)};
	}



	template<std::contiguous_iterator It>
	auto DeserializeMaterial(It& it, std::endian const endianness) -> Material
	{
		auto const diffuseColor{DeserializeColor(it)};
		auto const specularColor{DeserializeColor(it)};

		auto const gloss = Deserialize<f32>(it, endianness);
		auto const opacity = Deserialize<f32>(it, endianness);
		auto const twoSided = static_cast<bool>(Deserialize<u8>(it));
		auto const texFlags = Deserialize<u8>(it);

		std::optional<u64> diffuseMap;
		std::optional<u64> specularMap;
		std::optional<u64> opacityMap;

		if (texFlags & 0x80)
		{
			diffuseMap = Deserialize<u64>(it, endianness);
		}

		if (texFlags & 0x40)
		{
			specularMap = Deserialize<u64>(it, endianness);
		}

		if (texFlags & 0x20)
		{
			opacityMap = Deserialize<u64>(it, endianness);
		}

		return Material{diffuseColor, specularColor, gloss, opacity, twoSided, diffuseMap, specularMap, opacityMap};
	}



	template<std::contiguous_iterator It>
	auto DeserializeMesh(It& it, std::endian const endianness) -> Mesh
	{
		auto const numVertices = Deserialize<u64>(it, endianness);

		Mesh mesh;

		mesh.Vertices.reserve(numVertices);

		for (u64 i = 0; i < numVertices; i++)
		{
			mesh.Vertices.push_back(DeserializeVertex(it, endianness));
		}

		auto const numIndices = Deserialize<u64>(it, endianness);

		mesh.Indices.reserve(numIndices);

		for (u64 i = 0; i < numIndices; i++)
		{
			mesh.Indices.push_back(Deserialize<u32>(it, endianness));
		}

		mesh.Material = Deserialize<u64>(it, endianness);

		return mesh;
	}
}
