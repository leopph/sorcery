#pragma once

#include "MeshData.hpp"
#include "../shaders/ShaderProgram.hpp"

#include <glad/glad.h>

#include <array>
#include <cstddef>
#include <memory>


namespace leopph::internal
{
	/* GlMesh is the smallest unit of rendering.
	 * It acts as an opaque handle to OpenGL buffers ready for rendering.
	 * It also provides simple abstractions to make rendering easier.
	 * GlMesh instances only hold minimal data - copies share a single shared space in memory and keep reference counts. */
	class GlMesh final
	{
		public:
			GlMesh(const MeshData& meshData, unsigned instanceBuffer);

			GlMesh(const GlMesh& other) = delete;
			auto operator=(const GlMesh& other) -> GlMesh& = delete;

			GlMesh(GlMesh&& other) noexcept = delete;
			auto operator=(GlMesh&& other) noexcept -> GlMesh& = delete;

			~GlMesh();

			auto DrawWithMaterial(ShaderProgram& shader, GLuint nextFreeTextureUnit, GLsizei instanceCount) const -> void;
			auto DrawWithoutMaterial(GLsizei instanceCount) const -> void;

		private:
			constexpr static std::size_t VERTEX_BUFFER{0};
			constexpr static std::size_t INDEX_BUFFER{1};

			// Must not be null when in use!
			std::shared_ptr<Material> m_Material;
			GLuint m_VertexArray{0};
			std::array<GLuint, 2> m_Buffers{0, 0};
			GLsizei m_IndexCount{0};
			GLsizei m_VertexCount{0};
	};
}
