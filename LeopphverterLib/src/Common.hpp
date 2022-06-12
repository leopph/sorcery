#pragma once

#include <cstddef>
#include <string>
#include <vector>


namespace leopph::convert
{
	struct Material
	{
		float DiffuseClor[3]{1, 1, 1};
		float SpecularColor[3]{0, 0, 0};
		float Gloss{0};
		float Opacity{1};
		bool TwoSided{false};
		std::string DiffuseMap;
		std::string SpecularMap;
		std::string OpacityMap;
	};

	struct Vertex
	{
		float Position[3]{0, 0, 0};
		float Normal[3]{1, 0, 0};
		float TexCoord[2]{0, 0};
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
