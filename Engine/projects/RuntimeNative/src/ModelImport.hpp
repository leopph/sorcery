#pragma once

#include "AABB.hpp"
#include "Color.hpp"
#include "Image.hpp"

#include <bit>
#include <filesystem>
#include <memory>
#include <vector>

#include "Object.hpp"


namespace leopph {
	struct ObjectImportData {
		Guid guid{ Guid::Generate() };
	};


	struct MaterialImportData : ObjectImportData {
		Color albedo{ 255, 255, 255, 255 };
		f32 metallic{ 0 };
		f32 roughness{ 0.5f };
		f32 ao{ 1 };
	};


	struct Vertex {
		Vector3 position;
		Vector3 normal;
		Vector2 uv;
	};


	struct MeshImportData : ObjectImportData {
		std::vector<Vertex> vertices;
		std::vector<u32> indices;
		AABB bounds;
	};


	struct ModelImportData : ObjectImportData {
		std::vector<MeshImportData> meshes;
		std::vector<MaterialImportData> materials;
	};

	struct MeshData {
		std::vector<Vector3> positions;
		std::vector<Vector3> normals;
		std::vector<Vector2> uvs;
		std::vector<u32> indices;
	};


	[[nodiscard]] auto CalculateBounds(std::span<Vertex const> vertices) noexcept -> AABB;

	[[nodiscard]] auto ImportModelAsset(std::filesystem::path const& srcFile) -> ModelImportData;
	LEOPPHAPI [[nodiscard]] auto ImportMeshResourceData(std::filesystem::path const& srcPath) -> MeshData;
}
