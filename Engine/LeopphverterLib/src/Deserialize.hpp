#pragma once

#include "LeopphverterTypes.hpp"

#include <bit>
#include <cstdint>
#include <iterator>
#include <span>


namespace leopph::convert
{
	auto DeserializeI32(std::span<std::uint8_t const, 4> bytes, std::endian endianness) -> std::int32_t;
	auto DeserializeU32(std::span<std::uint8_t const, 4> bytes, std::endian endianness) -> std::uint32_t;

	auto DeserializeU64(std::span<std::uint8_t const, 8> bytes, std::endian endianness) -> std::uint64_t;

	auto DeserializeF32(std::span<std::uint8_t const, 4> bytes, std::endian endianness) -> float;

	auto DeserializeVec2(std::span<std::uint8_t const, sizeof(Vector2)> bytes, std::endian endianness) -> Vector2;
	auto DeserializeVec3(std::span<std::uint8_t const, sizeof(Vector3)> bytes, std::endian endianness) -> Vector3;

	auto DeserializeVertex(std::span<std::uint8_t const, sizeof(Vertex)> bytes, std::endian endianness) -> Vertex;

	template<std::contiguous_iterator It>
	auto DeserializeMaterial(It& it, std::endian endianness) -> Material;

	template<std::contiguous_iterator It>
	auto DeserializeMesh(It& it, std::endian endianness) -> Mesh;


	template<std::contiguous_iterator It>
	auto DeserializeMaterial(It& it, std::endian const endianness) -> Material
	{
		Color const diffuseColor{*it, *(it + 1), *(it + 2)};
		Color const specularColor{*(it + 3), *(it + 4), *(it + 5)};
		it += 6;

		auto const gloss = DeserializeF32(std::span<std::uint8_t const, 4>{it, 4}, endianness);
		auto const opacity = DeserializeF32(std::span<std::uint8_t const, 4>{it + 4, 4}, endianness);
		auto const twoSided = static_cast<bool>(*(it + 8));
		auto const texFlags = *(it + 9);
		it += 10;

		std::optional<std::size_t> diffuseMap;
		std::optional<std::size_t> specularMap;
		std::optional<std::size_t> opacityMap;

		if (texFlags & 0x80)
		{
			diffuseMap = DeserializeU64(std::span<std::uint8_t const, 8>{it, 8}, endianness);
		}

		if (texFlags & 0x40)
		{
			specularMap = DeserializeU64(std::span<std::uint8_t const, 8>{it + 8, 8}, endianness);
		}

		if (texFlags & 0x20)
		{
			opacityMap = DeserializeU64(std::span<std::uint8_t const, 8>{it + 16, 8}, endianness);
		}

		it += 24;
		return Material{diffuseColor, specularColor, gloss, opacity, twoSided, diffuseMap, specularMap, opacityMap};
	}


	template<std::contiguous_iterator It>
	auto DeserializeMesh(It& it, std::endian const endianness) -> Mesh
	{
		auto const numVertices = DeserializeU64(std::span<std::uint8_t const, 8>{it, 8}, endianness);
		it += 8;

		Mesh mesh;

		mesh.Vertices.reserve(numVertices);

		for (std::uint64_t i = 0; i < numVertices; i++)
		{
			mesh.Vertices.push_back(DeserializeVertex(std::span<std::uint8_t const, sizeof(Vertex)>{it, it + sizeof(Vertex)}, endianness));
			it += 32;
		}

		auto const numIndices = DeserializeU64(std::span<std::uint8_t const, 8>{it, 8}, endianness);
		it += 8;

		mesh.Indices.reserve(numIndices);

		for (std::uint64_t i = 0; i < numIndices; i++)
		{
			mesh.Indices.push_back(DeserializeU32(std::span<std::uint8_t const, 4>{it, it + 4}, endianness));
			it += 4;
		}

		mesh.Material = DeserializeU64(std::span<std::uint8_t const, 8>{it, it + 8}, endianness);
		it += 8;

		return mesh;
	}
}
