#pragma once

#include "GlMesh.hpp"
#include "../../math/Matrix.hpp"
#include "../geometry/MeshGroup.hpp"
#include "../opengl/GlBuffer.hpp"
#include "../opengl/OpenGl.hpp"
#include "../shaders/ShaderProgram.hpp"

#include <memory>
#include <vector>


namespace leopph::internal
{
	// Covers and renders a MeshGroup using multiple OpenGlMeshes.
	class GlMeshGroup
	{
		public:
			explicit GlMeshGroup(MeshGroup meshGroup);

			auto DrawWithMaterial(ShaderProgram& shader, GLuint nextFreeTextureUnit, bool transparent) const -> void;

			auto DrawWithoutMaterial(bool transparent) const -> void;

			// Loads the passed matrices into the instance buffer. Matrices must be in column major storage.
			auto SetInstanceData(std::vector<std::pair<Matrix4, Matrix4>> const& instMats) -> void;

			// Returns the MeshGroup that the GlMeshGroup currently mirrors.
			// If the MeshGroup changed after the construction of this object, the internal render buffers may not contain up to date data.
			// Use UpdateMeshData to reparse.
			[[nodiscard]]
			auto MeshGroup() const -> MeshGroup const&;

			// Sets the MeshGroup that the GlMeshGroup renders.
			// This automatically updates the internal render buffers which is an expensive operation.
			auto MeshGroup(leopph::MeshGroup meshGroup) -> void;

			// Reparses the data in the MeshGroup and recreates the internal representation of the Meshes.
			// This is an expensive operation.
			auto UpdateMeshData() -> void;

			// Separates transparent and opaque meshes.
			auto SortMeshes() -> void;

		private:
			[[nodiscard]] static
			auto FullyTransparent(std::shared_ptr<Material const> const& mat) -> bool;

			[[nodiscard]] static
			auto MaybeTransparent(std::shared_ptr<Material const> const& mat) -> bool;

		public:
			GlMeshGroup(GlMeshGroup const& other) = delete;
			auto operator=(GlMeshGroup const& other) -> GlMeshGroup& = delete;

			GlMeshGroup(GlMeshGroup&& other) noexcept = delete;
			auto operator=(GlMeshGroup&& other) noexcept -> GlMeshGroup& = delete;

			~GlMeshGroup() noexcept;

		private:
			std::vector<std::unique_ptr<GlMesh>> m_OpaqueMeshes;
			std::vector<std::unique_ptr<GlMesh>> m_MaybeTransparentMeshes;
			std::vector<std::unique_ptr<GlMesh>> m_FullyTransparentMeshes;
			leopph::MeshGroup m_MeshGroup;
			GlBuffer m_InstanceBuffer;
			GLsizei m_NumInstances{0};
	};
}
