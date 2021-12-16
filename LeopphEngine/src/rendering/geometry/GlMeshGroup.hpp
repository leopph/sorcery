#pragma once

#include "GlMesh.hpp"
#include "MeshDataGroup.hpp"
#include "../../math/Matrix.hpp"
#include "../shaders/ShaderProgram.hpp"

#include <cstddef>
#include <memory>
#include <utility>
#include <vector>


namespace leopph::impl
{
	/* GlMeshGroup holds together multiple GlMeshes that logically belong together (e.g. are part of the same model).
	 * It provides a simple abstraction over instanced rendering of the collected GlMeshes.
	 * GlMeshGroup instances hold minimal data - they share rources through a common memory area. */
	class GlMeshGroup final
	{
		public:
			explicit GlMeshGroup(std::shared_ptr<const MeshDataGroup> meshDataGroup);

			GlMeshGroup(const GlMeshGroup& other);
			GlMeshGroup& operator=(const GlMeshGroup& other);

			GlMeshGroup(GlMeshGroup&& other) noexcept;
			GlMeshGroup& operator=(GlMeshGroup&& other) noexcept;

			~GlMeshGroup() noexcept;
			
			void DrawShaded(ShaderProgram& shader, std::size_t nextFreeTextureUnit) const;
			void DrawDepth() const;

			// Loads the passed matrices into the instance buffer.
			void SetInstanceData(const std::vector<std::pair<Matrix4, Matrix4>>& instMats) const;

			[[nodiscard]]
			const MeshDataGroup& MeshData() const;

		private:
			void Deinit() const;

			struct SharedData
			{
				std::shared_ptr<const MeshDataGroup> MeshData{nullptr};
				std::vector<GlMesh> Meshes;
				unsigned InstBuf{0u};
				std::size_t InstBufSz{1ull};
				std::size_t InstCount{0ull};
				std::size_t RefCount{1ull};
			};


			std::shared_ptr<SharedData> m_SharedData;
	};
}
