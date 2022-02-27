#pragma once

#include "MeshData.hpp"
#include "../shaders/ShaderProgram.hpp"

#include <glad/gl.h>

#include <array>
#include <cstddef>
#include <memory>


namespace leopph::internal
{
	// The smallest unit of rendering.
	// A handle to a set of OpenGL buffers used for rendering.
	class GlMesh final
	{
		public:
			GlMesh(const MeshData& meshData, GLuint instanceBuffer);

			GlMesh(const GlMesh& other) = delete;
			auto operator=(const GlMesh& other) -> GlMesh& = delete;

			GlMesh(GlMesh&& other) noexcept = delete;
			auto operator=(GlMesh&& other) noexcept -> GlMesh& = delete;

			~GlMesh() noexcept;

			auto DrawWithMaterial(ShaderProgram& shader, GLuint nextFreeTextureUnit, GLsizei instanceCount) const -> void;
			auto DrawWithoutMaterial(GLsizei instanceCount) const -> void;

		private:
			// Must not be null when in use!
			std::shared_ptr<Material> m_Material;
			GLuint m_VertexArray{0};
			std::array<GLuint, 2> m_Buffers{0, 0};
			GLsizei m_NumIndices;

			constexpr static std::size_t VERTEX_BUFFER{0};
			constexpr static std::size_t INDEX_BUFFER{1};
	};
}
