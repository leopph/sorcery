#pragma once

#include "Color.hpp"
#include "Image.hpp"
#include "Vertex.hpp"

#include <cstddef>
#include <optional>
#include <vector>


namespace leopph::convert
{
	struct Material
	{
		Color DiffuseColor{255, 255, 255};
		Color SpecularColor{0, 0, 0};
		float Gloss{0};
		float Opacity{1};
		bool TwoSided{false};
		std::optional<std::size_t> DiffuseMap;
		std::optional<std::size_t> SpecularMap;
		std::optional<std::size_t> OpacityMap;
	};


	struct Mesh
	{
		std::vector<Vertex> Vertices;
		std::vector<unsigned> Indices;
		std::size_t Material;
	};


	struct Object
	{
		std::vector<Mesh> Meshes;
		std::vector<Material> Materials;
		std::vector<Image> Textures;
	};
}
