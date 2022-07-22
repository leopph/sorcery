#pragma once

#include "GlBufferObject.hpp"
#include "GlCore.hpp"
#include "GlVertexArrayObject.hpp"
#include "Mesh.hpp"
#include "rendering/shaders/ShaderFamily.hpp"


namespace leopph::internal
{
	// Covers and renders a Mesh.
	class GlMesh final
	{
		public:
			// instanceBuffer must be a valid buffer object.
			GlMesh(Mesh const& mesh, GLuint instanceBuffer);

			void DrawWithMaterial(ShaderFamily& shader, GLuint nextFreeTextureUnit, GLsizei instanceCount) const;

			void DrawWithoutMaterial(GLsizei instanceCount) const;

			// Returns the Material used for rendering.
			[[nodiscard]]
			std::shared_ptr<Material const> const& Material() const noexcept;

			GlMesh(GlMesh const& other) = delete;
			GlMesh& operator=(GlMesh const& other) = delete;

			GlMesh(GlMesh&& other) noexcept = delete;
			GlMesh& operator=(GlMesh&& other) noexcept = delete;

			~GlMesh() noexcept = default;

		private:
			GlVertexArrayObject m_VertexArray{};
			GlBufferObject m_VertexBuffer{};
			GlBufferObject m_IndexBuffer{};
			GLsizei m_NumIndices;
			std::shared_ptr<leopph::Material const> m_Material;
	};
}
