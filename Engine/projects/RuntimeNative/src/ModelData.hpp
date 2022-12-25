#pragma once

#include "AABB.hpp"
#include "Color.hpp"
#include "Image.hpp"
#include "Core.hpp"

#include <optional>
#include <vector>


namespace leopph {
	struct MaterialData {
		Color albedo{ 255, 255, 255, 255 };
		std::optional<std::size_t> albedoMapIndex;

		f32 metallic{ 0 };
		std::optional<std::size_t> metallicMapIndex;

		f32 roughness{ 0.5f };
		std::optional<std::size_t> roughnessMapIndex;

		f32 ao{ 1 };
		std::optional<std::size_t> aoMapIndex;
	};


	struct Vertex {
		Vector3 position;
		Vector3 normal;
		Vector2 uv;
	};


	struct MeshData {
		std::vector<Vertex> vertices;
		std::vector<u32> indices;
		AABB boundingBox;
	};


	struct ModelData {
		std::vector<MeshData> meshes;
		std::vector<MaterialData> materials;
		std::vector<Image> textures;
	};
}
