#pragma once

#include "Color.hpp"
#include "Image.hpp"
#include "Types.hpp"
#include "Vertex.hpp"

#include <optional>
#include <vector>


namespace leopph::convert
{
	struct Material
	{
		Color DiffuseColor{255, 255, 255};
		Color SpecularColor{0, 0, 0};
		f32 Gloss{0};
		f32 Opacity{1};
		bool TwoSided{false};
		std::optional<u64> DiffuseMap;
		std::optional<u64> SpecularMap;
		std::optional<u64> OpacityMap;
	};


	struct Mesh
	{
		std::vector<Vertex> Vertices;
		std::vector<unsigned> Indices;
		u64 Material;
	};


	struct Object
	{
		std::vector<Image> Textures;
		std::vector<Material> Materials;
		std::vector<Mesh> Meshes;
	};
}
