#pragma once

#include "Mesh.hpp"
#include "../shaders/ShaderProgram.hpp"

#include <glad/gl.h>


namespace leopph::internal
{
	// The smallest unit of rendering.
	// A handle to a set of OpenGL buffers used for rendering.
	class GlMesh final
	{
		public:
			// mesh must not be null and instanceBuffer must be a valid buffer object.
			GlMesh(Mesh const* mesh, GLuint instanceBuffer);

			auto DrawWithMaterial(ShaderProgram& shader, GLuint nextFreeTextureUnit, GLsizei instanceCount) const -> void;
			auto DrawWithoutMaterial(GLsizei instanceCount) const -> void;

			// Not null.
			[[nodiscard]]
			auto Mesh() const -> Mesh const*;

			// Shall not be null.
			auto Mesh(internal::Mesh const* mesh) -> void;

			GlMesh(GlMesh const& other) = delete;
			auto operator=(GlMesh const& other) -> GlMesh& = delete;

			GlMesh(GlMesh&& other) noexcept = delete;
			auto operator=(GlMesh&& other) noexcept -> GlMesh& = delete;

			~GlMesh() noexcept;

		private:
			// Recreates and configures the vertex and index buffers using the current Mesh.
			auto SetupBuffers() -> void;

			GLuint m_VertexArray{};
			GLuint m_VertexBuffer{};
			GLuint m_IndexBuffer{};
			internal::Mesh const* m_Mesh;
	};
}
