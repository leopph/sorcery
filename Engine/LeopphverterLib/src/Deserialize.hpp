#pragma once

#include "LeopphverterTypes.hpp"

#include <bit>
#include <cstdint>
#include <iterator>
#include <span>


namespace leopph::convert
{
	auto DeserializeI32(std::span<std::uint8_t const> bytes, std::endian endianness) -> std::int32_t;
	auto DeserializeU32(std::span<std::uint8_t const> bytes, std::endian endianness) -> std::uint32_t;

	auto DeserializeU64(std::span<std::uint8_t const> bytes, std::endian endianness) -> std::uint64_t;

	auto DeserializeF32(std::span<std::uint8_t const> bytes, std::endian endianness) -> float;

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


	template<std::contiguous_iterator It>
	auto DeserializeVec2(It& it, std::endian endianness) -> Vector2
	{
		Vector2 const ret
		{
			DeserializeF32({it, 4}, endianness),
			DeserializeF32({it + 4, 4}, endianness)
		};
		it += 8;
		return ret;
	}


	template<std::contiguous_iterator It>
	auto DeserializeVec3(It& it, std::endian endianness) -> Vector3
	{
		Vector3 const ret
		{
			DeserializeF32({it, 4}, endianness),
			DeserializeF32({it + 4, 4}, endianness),
			DeserializeF32({it + 8, 4}, endianness)
		};
		it += 12;
		return ret;
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
		Color const ret{*it, *(it + 1), *(it + 2)};
		it += 3;
		return ret;
	}


	template<std::contiguous_iterator It>
	auto DeserializeImage(It& it, std::endian const endianness) -> Image
	{
		auto const width = DeserializeI32({it, 4}, endianness);
		auto const height = DeserializeI32({it + 4, 4}, endianness);
		auto const chans = *(it + 8);
		it += 9;

		auto const imgSz = width * height * chans;
		auto imgData = std::make_unique<unsigned char[]>(imgSz);
		std::copy_n(it, imgSz, imgData.get());
		it += imgSz;
		return Image{width, height, chans, std::move(imgData)};
	}


	template<std::contiguous_iterator It>
	auto DeserializeMaterial(It& it, std::endian const endianness) -> Material
	{
		auto const diffuseColor{DeserializeColor(it)};
		auto const specularColor{DeserializeColor(it)};

		auto const gloss = DeserializeF32({it, 4}, endianness);
		auto const opacity = DeserializeF32({it + 4, 4}, endianness);
		auto const twoSided = static_cast<bool>(*(it + 8));
		auto const texFlags = *(it + 9);
		it += 10;

		std::optional<std::size_t> diffuseMap;
		std::optional<std::size_t> specularMap;
		std::optional<std::size_t> opacityMap;

		if (texFlags & 0x80)
		{
			diffuseMap = DeserializeU64({it, 8}, endianness);
		}

		if (texFlags & 0x40)
		{
			specularMap = DeserializeU64({it + 8, 8}, endianness);
		}

		if (texFlags & 0x20)
		{
			opacityMap = DeserializeU64({it + 16, 8}, endianness);
		}

		it += 24;
		return Material{diffuseColor, specularColor, gloss, opacity, twoSided, diffuseMap, specularMap, opacityMap};
	}


	template<std::contiguous_iterator It>
	auto DeserializeMesh(It& it, std::endian const endianness) -> Mesh
	{
		auto const numVertices = DeserializeU64({it, 8}, endianness);
		it += 8;

		Mesh mesh;

		mesh.Vertices.reserve(numVertices);

		for (std::uint64_t i = 0; i < numVertices; i++)
		{
			mesh.Vertices.push_back(DeserializeVertex(it, endianness));
		}

		auto const numIndices = DeserializeU64({it, 8}, endianness);
		it += 8;

		mesh.Indices.reserve(numIndices);

		for (std::uint64_t i = 0; i < numIndices; i++)
		{
			mesh.Indices.push_back(DeserializeU32({it, it + 4}, endianness));
			it += 4;
		}

		mesh.Material = DeserializeU64({it, it + 8}, endianness);
		it += 8;

		return mesh;
	}
}
