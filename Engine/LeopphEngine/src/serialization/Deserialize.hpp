#pragma once

#include "Leopphverter.hpp"
#include "Types.hpp"
#include "Vector.hpp"
#include "Vertex.hpp"

#include <array>
#include <bit>
#include <iterator>
#include <type_traits>


namespace leopph
{
	template<typename T, std::contiguous_iterator It>
		requires std::is_scalar_v<T> && (sizeof(T) == 1)
	T deserialize(It const& it);

	template<typename T, std::contiguous_iterator It>
		requires std::is_scalar_v<T> && (sizeof(T) == 1)
	T deserialize(It& it);

	template<typename T, std::contiguous_iterator It>
		requires std::is_scalar_v<T> && (sizeof(T) > 1)
	T deserialize(It const& it, std::endian endianness);

	template<typename T, std::contiguous_iterator It>
		requires std::is_scalar_v<T> && (sizeof(T) > 1)
	T deserialize(It& it, std::endian endianness);


	template<std::contiguous_iterator It>
	Vector2 deserialize_vector2(It& it, std::endian endianness);

	template<std::contiguous_iterator It>
	Vector3 deserialize_vector3(It& it, std::endian endianness);


	template<std::contiguous_iterator It>
	Color deserialize_color(It& it);


	template<std::contiguous_iterator It>
	Image deserialize_image(It& it, std::endian endianness);


	template<std::contiguous_iterator It>
	Vertex deserialize_vertex(It& it, std::endian endianness);

	template<std::contiguous_iterator It>
	Material deserialize_material(It& it, std::endian endianness);

	template<std::contiguous_iterator It>
	Mesh deserialize_mesh(It& it, std::endian endianness);



	template<typename T, std::contiguous_iterator It>
		requires std::is_scalar_v<T> && (sizeof(T) == 1)
	T deserialize(It const& it)
	{
		return *reinterpret_cast<T const*>(&*it);
	}



	template<typename T, std::contiguous_iterator It>
		requires std::is_scalar_v<T> && (sizeof(T) == 1)
	T deserialize(It& it)
	{
		auto const ret = deserialize<T>(const_cast<It const&>(it));
		it += 1;
		return ret;
	}



	template<typename T, std::contiguous_iterator It>
		requires std::is_scalar_v<T> && (sizeof(T) > 1)
	T deserialize(It const& it, std::endian const endianness)
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



	template<typename T, std::contiguous_iterator It>
		requires std::is_scalar_v<T> && (sizeof(T) > 1)
	T deserialize(It& it, std::endian const endianness)
	{
		auto const ret = deserialize<T>(const_cast<It const&>(it), endianness);
		it += sizeof(T);
		return ret;
	}



	template<std::contiguous_iterator It>
	Vector2 deserialize_vector2(It& it, std::endian endianness)
	{
		return Vector2
		{
			deserialize<f32>(it, endianness),
			deserialize<f32>(it, endianness)
		};
	}



	template<std::contiguous_iterator It>
	Vector3 deserialize_vector3(It& it, std::endian endianness)
	{
		return Vector3
		{
			deserialize<f32>(it, endianness),
			deserialize<f32>(it, endianness),
			deserialize<f32>(it, endianness)
		};
	}



	template<std::contiguous_iterator It>
	Color deserialize_color(It& it)
	{
		return Color
		{
			deserialize<u8>(it),
			deserialize<u8>(it),
			deserialize<u8>(it)
			// TODO DESERIALIZE ALPHA
		};
	}



	template<std::contiguous_iterator It>
	Vertex deserialize_vertex(It& it, std::endian const endianness)
	{
		return Vertex
		{
			.position = deserialize_vector3(it, endianness),
			.normal = deserialize_vector3(it, endianness),
			.uv = deserialize_vector2(it, endianness)
		};
	}



	template<std::contiguous_iterator It>
	Image deserialize_image(It& it, std::endian const endianness)
	{
		auto const width = deserialize<u32>(it, endianness);
		auto const height = deserialize<u32>(it, endianness);
		auto const chans = deserialize<u8>(it);
		auto const encoding = deserialize<u8>(it);

		auto imgSz = static_cast<u64>(width) * height * chans;
		auto imgData = std::make_unique<unsigned char[]>(imgSz);
		std::copy_n(it, width * height * chans, imgData.get());
		it += imgSz;

		return Image{width, height, chans, std::move(imgData), static_cast<ColorEncoding>(encoding)};
	}



	template<std::contiguous_iterator It>
	Material deserialize_material(It& it, std::endian const endianness)
	{
		auto const diffuseColor{deserialize_color(it)};
		auto const specularColor{deserialize_color(it)};

		auto const gloss = deserialize<f32>(it, endianness);
		auto const opacity = deserialize<f32>(it, endianness);
		auto const twoSided = static_cast<bool>(deserialize<u8>(it));
		auto const texFlags = deserialize<u8>(it);

		std::optional<u64> diffuseMap;
		std::optional<u64> specularMap;
		std::optional<u64> opacityMap;

		if (texFlags & 0x80)
		{
			diffuseMap = deserialize<u64>(it, endianness);
		}

		if (texFlags & 0x40)
		{
			specularMap = deserialize<u64>(it, endianness);
		}

		if (texFlags & 0x20)
		{
			opacityMap = deserialize<u64>(it, endianness);
		}

		return Material{diffuseColor, specularColor, gloss, opacity, twoSided, diffuseMap, specularMap, opacityMap};
	}



	template<std::contiguous_iterator It>
	Mesh deserialize_mesh(It& it, std::endian const endianness)
	{
		auto const numVertices = deserialize<u64>(it, endianness);

		Mesh mesh;

		mesh.vertices.reserve(numVertices);

		for (u64 i = 0; i < numVertices; i++)
		{
			mesh.vertices.push_back(deserialize_vertex(it, endianness));
		}

		auto const numIndices = deserialize<u64>(it, endianness);

		mesh.indices.reserve(numIndices);

		for (u64 i = 0; i < numIndices; i++)
		{
			mesh.indices.push_back(deserialize<u32>(it, endianness));
		}

		mesh.material = deserialize<u64>(it, endianness);

		return mesh;
	}
}
