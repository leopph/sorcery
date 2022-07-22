#pragma once

#include "Color.hpp"
#include "Image.hpp"
#include "LeopphApi.hpp"
#include "Types.hpp"
#include "Vertex.hpp"

#include <bit>
#include <filesystem>
#include <optional>
#include <vector>


namespace leopph::convert
{
	struct Material
	{
		Color diffuseColor{255, 255, 255};
		Color specularColor{0, 0, 0};
		f32 gloss{0};
		f32 opacity{1};
		bool twoSided{false};
		std::optional<u64> diffuseMap;
		std::optional<u64> specularMap;
		std::optional<u64> opacityMap;
	};


	struct Mesh
	{
		std::vector<Vertex> vertices;
		std::vector<unsigned> indices;
		u64 material;
	};


	struct Object
	{
		std::vector<Image> textures;
		std::vector<Material> materials;
		std::vector<Mesh> meshes;
	};


	LEOPPHAPI std::vector<unsigned char> encode_in_leopph3d(Object const& object, std::endian endianness = std::endian::native);
	LEOPPHAPI std::optional<Object> import_3d_asset(std::filesystem::path const& path);
}
