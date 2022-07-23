#pragma once

#include "GlBufferObject.hpp"
#include "GlCore.hpp"
#include "GlMesh.hpp"
#include "Matrix.hpp"
#include "MeshGroup.hpp"
#include "RenderObject.hpp"
#include "ShaderFamily.hpp"

#include <gsl/pointers>

#include <memory>
#include <span>
#include <vector>


namespace leopph::internal
{
	class GlMeshGroup final : public RenderObject
	{
		public:
			explicit GlMeshGroup(MeshGroup meshGroup);


			void draw_with_material(gsl::not_null<ShaderProgram const*> shader, GLuint nextFreeTextureUnit, bool transparent) const;
			void draw_without_material(bool transparent) const;


			// Matrices must be in column major storage.
			void set_instance_data(std::span<std::pair<Matrix4, Matrix4> const> instMats);


			[[nodiscard]] MeshGroup const& get_mesh_group() const;
			void set_mesh_group(MeshGroup meshGroup);
			void update_mesh_data();


			void sort_meshes();


		private:
			[[nodiscard]] static bool is_guaranteed_transparent(std::shared_ptr<Material const> const& mat);
			[[nodiscard]] static bool is_potentially_transparent(std::shared_ptr<Material const> const& mat);


		public:
			GlMeshGroup(GlMeshGroup const& other) = delete;
			GlMeshGroup& operator=(GlMeshGroup const& other) = delete;

			GlMeshGroup(GlMeshGroup&& other) noexcept = delete;
			GlMeshGroup& operator=(GlMeshGroup&& other) noexcept = delete;

			~GlMeshGroup() noexcept override = default;


		private:
			std::vector<std::unique_ptr<GlMesh>> mOpaqueMeshes;
			std::vector<std::unique_ptr<GlMesh>> mPotentiallyTransparentMeshes;
			std::vector<std::unique_ptr<GlMesh>> mGuaranteedTransparentMeshes;
			MeshGroup mMeshGroup;
			GlBufferObject mInstanceBuffer;
			GLsizei mInstanceBufferCapacity{1};	// The maximum number of elements the buffer can hold
			GLsizei mInstanceBufferSize{0};		// The number of elements in the buffer
	};
}
