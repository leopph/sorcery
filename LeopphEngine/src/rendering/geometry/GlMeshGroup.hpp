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

			GlMeshGroup(const GlMeshGroup& other) = delete;
			auto operator=(const GlMeshGroup& other) -> GlMeshGroup& = delete;

			GlMeshGroup(GlMeshGroup&& other) noexcept = delete;
			auto operator=(GlMeshGroup&& other) noexcept -> GlMeshGroup& = delete;

			~GlMeshGroup() noexcept;

			auto DrawShaded(ShaderProgram& shader, std::size_t nextFreeTextureUnit) const -> void;
			auto DrawDepth() const -> void;

			// Loads the passed matrices into the instance buffer. Matrices must be in column major storage.
			auto SetInstanceData(const std::vector<std::pair<Matrix4, Matrix4>>& instMats) const -> void;

			[[nodiscard]]
			auto MeshData() const -> const MeshDataGroup&;

		private:
			std::shared_ptr<const MeshDataGroup> m_MeshData{nullptr};
			std::vector<GlMesh> m_Meshes;
			unsigned m_InstBuf{0u};
			mutable std::size_t m_InstBufSz{1ull};
			mutable std::size_t m_InstCount{0ull};
	};
}
