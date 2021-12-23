#pragma once

#include "GlMesh.hpp"
#include "MeshDataGroup.hpp"
#include "../../math/Matrix.hpp"
#include "../shaders/ShaderProgram.hpp"

#include <cstddef>
#include <memory>
#include <utility>
#include <vector>


namespace leopph::internal
{
	/* GlMeshGroup holds together multiple GlMeshes that logically belong together (e.g. are part of the same model).
	 * It provides a simple abstraction over instanced rendering of the collected GlMeshes.
	 * GlMeshGroup instances hold minimal data - they share rources through a common memory area. */
	class GlMeshGroup final
	{
		public:
			explicit GlMeshGroup(std::shared_ptr<const MeshDataGroup> meshDataGroup);

			GlMeshGroup(const GlMeshGroup& other);
			auto operator=(const GlMeshGroup& other) -> GlMeshGroup&;

			GlMeshGroup(GlMeshGroup&& other) noexcept;
			auto operator=(GlMeshGroup&& other) noexcept -> GlMeshGroup&;

			~GlMeshGroup() noexcept;

			auto DrawShaded(ShaderProgram& shader, std::size_t nextFreeTextureUnit) const -> void;
			auto DrawDepth() const -> void;

			// Loads the passed matrices into the instance buffer.
			auto SetInstanceData(const std::vector<std::pair<Matrix4, Matrix4>>& instMats) const -> void;

			[[nodiscard]]
			auto MeshData() const -> const MeshDataGroup&;

		private:
			auto Deinit() const -> void;


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
