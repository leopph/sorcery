#pragma once

#include "Material.hpp"
#include "Types.hpp"
#include "Vertex.hpp"

#include <memory>
#include <span>
#include <vector>


namespace leopph
{
	class StaticMesh
	{
		public:
			struct SubMesh
			{
				u32 first;
				u32 count;
			};


			[[nodiscard]] std::span<SubMesh const> get_sub_meshes() const;

			[[nodiscard]] std::span<std::shared_ptr<Material const> const> get_materials() const;
			void set_materials(std::vector<std::shared_ptr<Material const>> materials);

			void set_vertices(std::span<Vertex const> vertices);
			void set_indices(std::span<u32 const> indices);

			StaticMesh();

			StaticMesh(StaticMesh const&) = delete;
			void operator=(StaticMesh const&) = delete;

			StaticMesh(StaticMesh&&) = delete;
			void operator=(StaticMesh&&) = delete;

			~StaticMesh();

		private:
			std::vector<SubMesh> mSubMeshes;
			std::vector<std::shared_ptr<Material const>> mMaterials;
			u32 mVao;
			u32 mVbo;
			u32 mIbo;
	};
}
