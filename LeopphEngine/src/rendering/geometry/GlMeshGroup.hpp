#pragma once

#include "GlMesh.hpp"
#include "../../math/Matrix.hpp"
#include "../geometry/MeshGroup.hpp"
#include "../shaders/ShaderProgram.hpp"

#include <glad/gl.h>

#include <memory>
#include <utility>
#include <vector>


namespace leopph::internal
{
	// Holds together multiple OpenGlMeshes that logically belong together (e.g. are part of the same model).
	class GlMeshGroup
	{
		public:
			// Returns the GlMeshGroup that uses the passed MeshGroup, or creates a new if there is none.
			[[nodiscard]] static
			auto CreateOrGet(std::shared_ptr<MeshGroup const>&& meshGroup) -> std::shared_ptr<GlMeshGroup>;

		private:
			explicit GlMeshGroup(std::shared_ptr<MeshGroup const>&& meshGroup);

		public:
			auto DrawWithMaterial(ShaderProgram& shader, GLuint nextFreeTextureUnit, bool transparent) const -> void;
			auto DrawWithoutMaterial(bool transparent) const -> void;

			// Loads the passed matrices into the instance buffer. Matrices must be in column major storage.
			auto SetInstanceData(std::vector<std::pair<Matrix4, Matrix4>> const& instMats) -> void;

			[[nodiscard]]
			auto MeshGroup() const -> std::shared_ptr<MeshGroup const> const&;

			// Separates transparent and opaque meshes.
			auto SortMeshes() -> void;

		private:
			// Determines whether a Mesh is semi-transparent.
			[[nodiscard]] static
			auto IsSemiTransparent(Mesh const& mesh) -> bool;

			// Determines if a Mesh is transparent.
			[[nodiscard]] static
			auto IsTransparent(Mesh const& mesh) -> bool;

		public:
			GlMeshGroup(GlMeshGroup const& other) = delete;
			auto operator=(GlMeshGroup const& other) -> GlMeshGroup& = delete;

			GlMeshGroup(GlMeshGroup&& other) noexcept = delete;
			auto operator=(GlMeshGroup&& other) noexcept -> GlMeshGroup& = delete;

			~GlMeshGroup() noexcept;

		private:
			// Only rendered as part of opaque pass.
			std::vector<std::unique_ptr<GlMesh>> m_OpaqueMeshes;
			// Rendered during the opaque as well as the transparent pass.
			// Shader's job to handle this.
			std::vector<std::unique_ptr<GlMesh>> m_SemiTransparentMeshes;
			// Only rendered as part of the transparent pass.
			std::vector<std::unique_ptr<GlMesh>> m_TransparentMeshes;
			std::shared_ptr<internal::MeshGroup const> m_MeshGroup;
			GLuint m_InstanceBuffer{0};
			int m_NumInstances{0};
	};
}
