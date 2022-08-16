#pragma once

#include "AABB.hpp"
#include "Color.hpp"
#include "Image.hpp"
#include "Types.hpp"
#include "Vertex.hpp"

#include <optional>
#include <vector>


namespace leopph
{
	struct MaterialData
	{
		// RGB controls the diffuse color, A controls the opacity.
		// For opaque surfaces, the opacity only takes part in alpha clipping,
		// while for transparent ones, it also affects blending.
		Color diffuseColor{255, 255, 255, 255};

		// RGB is multiplied with diffuseColor RGB, A is multiplied with the color alpha and used accordingly, if present.
		std::optional<std::size_t> diffuseMapIndex;

		// RGB controls the color of the specular highlights, A is unused.
		Color specularColor{0, 0, 0};

		// RGB is multiplied with specularColor RGB, A is unused if present.
		std::optional<std::size_t> specularMapIndex;

		// Controls the spread of the specular highlight.
		f32 gloss{0};

		// Discards pixels whose alpha values are under this value.
		f32 alphaThreshold{1.f};

		bool cullBackFace{true};
		bool isTransparent{false};
	};


	struct StaticMeshData
	{
		std::vector<Vertex> vertices;
		std::vector<u32> indices;
		AABB boundingBox;
	};


	struct StaticModelData
	{
		std::vector<StaticMeshData> meshes;
		std::vector<MaterialData> materials;
		std::vector<Image> textures;
	};
}
