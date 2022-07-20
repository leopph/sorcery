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

			auto DrawWithMaterial(ShaderFamily const& shader, GLuint nextFreeTextureUnit, GLsizei instanceCount) const -> void;

			auto DrawWithoutMaterial(GLsizei instanceCount) const -> void;

			// Returns the Material used for rendering.
			[[nodiscard]]
			auto Material() const noexcept -> std::shared_ptr<Material const> const&;

			GlMesh(GlMesh const& other) = delete;
			auto operator=(GlMesh const& other) -> GlMesh& = delete;

			GlMesh(GlMesh&& other) noexcept = delete;
			auto operator=(GlMesh&& other) noexcept -> GlMesh& = delete;

			~GlMesh() noexcept = default;

		private:
			GlVertexArrayObject m_VertexArray{};
			GlBufferObject m_VertexBuffer{};
			GlBufferObject m_IndexBuffer{};
			GLsizei m_NumIndices;
			std::shared_ptr<leopph::Material const> m_Material;
	};
}
