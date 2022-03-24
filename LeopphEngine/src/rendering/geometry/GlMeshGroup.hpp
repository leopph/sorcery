#pragma once

#include "GlMesh.hpp"
#include "../../math/Matrix.hpp"
#include "../geometry/MeshGroup.hpp"
#include "../shaders/ShaderProgram.hpp"

#include <GL/gl3w.h>

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
			std::shared_ptr<internal::MeshGroup const> m_MeshGroup;
			GLuint m_InstanceBuffer{0};
			int m_NumInstances{0};
	};
}
