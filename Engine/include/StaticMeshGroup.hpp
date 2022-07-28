#pragma once

#include "Material.hpp"
#include "Types.hpp"
#include "Vertex.hpp"

#include <memory>
#include <span>
#include <vector>


namespace leopph
{
	class StaticMeshGroup
	{
		public:
			class StaticMesh
			{
				public:
					StaticMesh(std::span<Vertex const> vertices, std::span<u32> indices);

					void draw() const;

					StaticMesh(StaticMesh const&) = delete;
					void operator=(StaticMesh const&) = delete;

					StaticMesh(StaticMesh&&) = delete;
					void operator=(StaticMesh&&) = delete;

					~StaticMesh();

				private:
					u32 mVao;
					u32 mVbo;
					u32 mIbo;
					u64 mNumIndices;
			};


			explicit StaticMeshGroup(std::vector<std::unique_ptr<StaticMesh>> meshes, std::vector<std::shared_ptr<Material>> materials);

			[[nodiscard]] std::span<std::unique_ptr<StaticMesh> const> get_meshes() const;
			[[nodiscard]] std::span<std::shared_ptr<Material> const> get_materials() const;

		private:
			std::vector<std::unique_ptr<StaticMesh>> mMeshes;
			std::vector<std::shared_ptr<Material>> mMaterials;
	};
}
