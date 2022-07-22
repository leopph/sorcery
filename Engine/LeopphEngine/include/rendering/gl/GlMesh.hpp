#pragma once

#include "GlBufferObject.hpp"
#include "GlCore.hpp"
#include "GlVertexArrayObject.hpp"
#include "Mesh.hpp"
#include "rendering/shaders/ShaderProgram.hpp"

#include <gsl/pointers>


namespace leopph::internal
{
	class GlMesh final
	{
		public:
			// instanceBuffer must be a valid buffer object.
			GlMesh(Mesh const& mesh, GLuint instanceBuffer);


			void draw_with_material(gsl::not_null<ShaderProgram const*> shader, GLuint nextFreeTextureUnit, GLsizei instanceCount) const;
			void draw_without_material(GLsizei instanceCount) const;


			[[nodiscard]]
			std::shared_ptr<Material const> const& get_material() const noexcept;


			GlMesh(GlMesh const& other) = delete;
			GlMesh& operator=(GlMesh const& other) = delete;

			GlMesh(GlMesh&& other) noexcept = delete;
			GlMesh& operator=(GlMesh&& other) noexcept = delete;

			~GlMesh() noexcept = default;


		private:
			GlVertexArrayObject mVao{};
			GlBufferObject mVbo{};
			GlBufferObject mIbo{};
			GLsizei mNumIndices;
			std::shared_ptr<Material const> mMaterial;
	};
}
