#pragma once

#include "../../LeopphEngine/src/math/Vector.hpp"

#include <cstddef>
#include <string>
#include <vector>


namespace leopph::convert
{
	struct Material
	{
		Vector3 DiffuseClor{};
		Vector3 SpecularColor{};
		float Gloss{0};
		float Opacity{1};
		bool TwoSided{false};
		std::string DiffuseMap;
		std::string SpecularMap;
		std::string OpacityMap;
	};


	struct Vertex
	{
		Vector3 Position{};
		Vector3 Normal{1, 0, 0};
		Vector2 TexCoord{};
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
	};
}
