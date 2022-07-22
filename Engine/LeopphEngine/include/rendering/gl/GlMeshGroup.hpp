#pragma once

#include "GlBufferObject.hpp"
#include "GlCore.hpp"
#include "GlMesh.hpp"
#include "Matrix.hpp"
#include "MeshGroup.hpp"
#include "rendering/RenderObject.hpp"
#include "rendering/shaders/ShaderFamily.hpp"

#include <memory>
#include <span>
#include <vector>


namespace leopph::internal
{
	// Covers and renders a MeshGroup using multiple OpenGlMeshes.
	class GlMeshGroup : public RenderObject
	{
		public:
			explicit GlMeshGroup(MeshGroup meshGroup);

			void DrawWithMaterial(ShaderFamily& shader, GLuint nextFreeTextureUnit, bool transparent) const;

			void DrawWithoutMaterial(bool transparent) const;

			// Loads the passed matrices into the instance buffer. Matrices must be in column major storage.
			void SetInstanceData(std::span<std::pair<Matrix4, Matrix4> const> instMats);

			// Returns the MeshGroup that the GlMeshGroup currently mirrors.
			// If the MeshGroup changed after the construction of this object, the internal render buffers may not contain up to date data.
			// Use UpdateMeshData to reparse.
			[[nodiscard]]
			MeshGroup const& MeshGroup() const;

			// Sets the MeshGroup that the GlMeshGroup renders.
			// This automatically updates the internal render buffers which is an expensive operation.
			void MeshGroup(leopph::MeshGroup meshGroup);

			// Reparses the data in the MeshGroup and recreates the internal representation of the Meshes.
			// This is an expensive operation.
			void UpdateMeshData();

			// Separates transparent and opaque meshes.
			void SortMeshes();

		private:
			[[nodiscard]] static bool FullyTransparent(std::shared_ptr<Material const> const& mat);

			[[nodiscard]] static bool MaybeTransparent(std::shared_ptr<Material const> const& mat);

		public:
			GlMeshGroup(GlMeshGroup const& other) = delete;
			GlMeshGroup& operator=(GlMeshGroup const& other) = delete;

			GlMeshGroup(GlMeshGroup&& other) noexcept = delete;
			GlMeshGroup& operator=(GlMeshGroup&& other) noexcept = delete;

			~GlMeshGroup() noexcept override = default;

		private:
			std::vector<std::unique_ptr<GlMesh>> m_OpaqueMeshes;
			std::vector<std::unique_ptr<GlMesh>> m_MaybeTransparentMeshes;
			std::vector<std::unique_ptr<GlMesh>> m_FullyTransparentMeshes;
			leopph::MeshGroup m_MeshGroup;
			GlBufferObject m_InstanceBuffer;
			GLsizei m_InstanceBufferSize{1};
			GLsizei m_NumInstances{0};
	};
}
