#include "GlMesh.hpp"

#include "../../math/Matrix.hpp"
#include "../../math/Vector.hpp"

#include <algorithm>
#include <cstddef>
#include <limits>
#include <utility>


namespace leopph::internal
{
	GlMesh::GlMesh(const MeshData& meshData, const GLuint instanceBuffer) :
		m_Material{meshData.Material},
		m_NumIndices{static_cast<decltype(m_NumIndices)>(std::clamp<decltype(meshData.Indices)::size_type>(meshData.Indices.size(), 0, std::numeric_limits<decltype(m_NumIndices)>::max()))}
	{
		// Clamp vertex and index container sizes to the max supported by OpenGL
		const auto numVertices{static_cast<GLsizei>(std::clamp<decltype(meshData.Vertices)::size_type>(meshData.Vertices.size(), 0, std::numeric_limits<GLsizei>::max()))};
		const auto vertBufSz{static_cast<GLsizei>(std::clamp<std::size_t>(numVertices * sizeof(decltype(meshData.Vertices)::value_type), 0, std::numeric_limits<GLsizei>::max()))};
		const auto indBufSz{static_cast<GLsizei>(std::clamp<std::size_t>(m_NumIndices * sizeof(decltype(meshData.Indices)::value_type), 0, std::numeric_limits<GLsizei>::max()))};

		// Setup buffers
		glCreateBuffers(static_cast<GLsizei>(m_Buffers.size()), m_Buffers.data());
		glNamedBufferStorage(m_Buffers[VERTEX_BUFFER], vertBufSz, meshData.Vertices.data(), 0);
		glNamedBufferStorage(m_Buffers[INDEX_BUFFER], indBufSz, meshData.Indices.data(), 0);

		// Add buffers to Vertex Array
		glCreateVertexArrays(1, &m_VertexArray);
		glVertexArrayVertexBuffers(m_VertexArray, 0, 2, std::array{m_Buffers[VERTEX_BUFFER], instanceBuffer}.data(), std::array<GLintptr, 2>{0, 0}.data(), std::array<GLsizei, 2>{sizeof(decltype(meshData.Vertices)::value_type), sizeof(std::pair<Matrix4, Matrix4>)}.data());
		glVertexArrayElementBuffer(m_VertexArray, m_Buffers[INDEX_BUFFER]);

		// Specify attributes in Vertex Array

		// Position
		glVertexArrayAttribFormat(m_VertexArray, 0, 3, GL_FLOAT, GL_FALSE, offsetof(decltype(meshData.Vertices)::value_type, Position));
		glVertexArrayAttribBinding(m_VertexArray, 0, 0);
		glEnableVertexArrayAttrib(m_VertexArray, 0);

		// Normal
		glVertexArrayAttribFormat(m_VertexArray, 1, 3, GL_FLOAT, GL_FALSE, offsetof(decltype(meshData.Vertices)::value_type, Normal));
		glVertexArrayAttribBinding(m_VertexArray, 1, 0);
		glEnableVertexArrayAttrib(m_VertexArray, 1);

		// Texture coordinates
		glVertexArrayAttribFormat(m_VertexArray, 2, 2, GL_FLOAT, GL_FALSE, offsetof(decltype(meshData.Vertices)::value_type, TexCoord));
		glVertexArrayAttribBinding(m_VertexArray, 2, 0);
		glEnableVertexArrayAttrib(m_VertexArray, 2);

		// Model matrix 1st row
		glVertexArrayAttribFormat(m_VertexArray, 3, 4, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribBinding(m_VertexArray, 3, 1);
		glEnableVertexArrayAttrib(m_VertexArray, 3);

		// Model matrix 2nd row
		glVertexArrayAttribFormat(m_VertexArray, 4, 4, GL_FLOAT, GL_FALSE, sizeof(Vector4));
		glVertexArrayAttribBinding(m_VertexArray, 4, 1);
		glEnableVertexArrayAttrib(m_VertexArray, 4);

		// Model matrix 3rd row
		glVertexArrayAttribFormat(m_VertexArray, 5, 4, GL_FLOAT, GL_FALSE, 2 * sizeof(Vector4));
		glVertexArrayAttribBinding(m_VertexArray, 5, 1);
		glEnableVertexArrayAttrib(m_VertexArray, 5);

		// Model matrix 4th row
		glVertexArrayAttribFormat(m_VertexArray, 6, 4, GL_FLOAT, GL_FALSE, 3 * sizeof(Vector4));
		glVertexArrayAttribBinding(m_VertexArray, 6, 1);
		glEnableVertexArrayAttrib(m_VertexArray, 6);

		// Normal matrix 1st row
		glVertexArrayAttribFormat(m_VertexArray, 7, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(Vector4));
		glVertexArrayAttribBinding(m_VertexArray, 7, 1);
		glEnableVertexArrayAttrib(m_VertexArray, 7);

		// Normal matrix 2nd row
		glVertexArrayAttribFormat(m_VertexArray, 8, 4, GL_FLOAT, GL_FALSE, 5 * sizeof(Vector4));
		glVertexArrayAttribBinding(m_VertexArray, 8, 1);
		glEnableVertexArrayAttrib(m_VertexArray, 8);

		// Normal matrix 3rd row
		glVertexArrayAttribFormat(m_VertexArray, 9, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(Vector4));
		glVertexArrayAttribBinding(m_VertexArray, 9, 1);
		glEnableVertexArrayAttrib(m_VertexArray, 9);

		// Normal matrix 4th row
		glVertexArrayAttribFormat(m_VertexArray, 10, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(Vector4));
		glVertexArrayAttribBinding(m_VertexArray, 10, 1);
		glEnableVertexArrayAttrib(m_VertexArray, 10);

		glVertexArrayBindingDivisor(m_VertexArray, 1, 1);
	}


	GlMesh::~GlMesh() noexcept
	{
		glDeleteVertexArrays(1, &m_VertexArray);
		glDeleteBuffers(static_cast<GLsizei>(m_Buffers.size()), m_Buffers.data());
	}


	auto GlMesh::DrawWithMaterial(ShaderProgram& shader, GLuint nextFreeTextureUnit, const GLsizei instanceCount) const -> void
	{
		shader.SetUniform("u_Material.diffuseColor", static_cast<Vector3>(m_Material->DiffuseColor));
		shader.SetUniform("u_Material.specularColor", static_cast<Vector3>(m_Material->SpecularColor));
		shader.SetUniform("u_Material.gloss", m_Material->Gloss);

		if (m_Material->DiffuseMap != nullptr)
		{
			shader.SetUniform("u_Material.hasDiffuseMap", true);
			shader.SetUniform("u_Material.diffuseMap", static_cast<GLint>(nextFreeTextureUnit)); /* cast to GLint because only glUniform1i[v] may be used to set sampler uniforms (wtf?) */
			glBindTextureUnit(nextFreeTextureUnit, m_Material->DiffuseMap->Id());
			++nextFreeTextureUnit;
		}
		else
		{
			shader.SetUniform("u_Material.hasDiffuseMap", false);
		}

		if (m_Material->SpecularMap != nullptr)
		{
			shader.SetUniform("u_Material.hasSpecularMap", true);
			shader.SetUniform("u_Material.specularMap", static_cast<GLint>(nextFreeTextureUnit)); /* cast to GLint because only glUniform1i[v] may be used to set sampler uniforms (wtf?) */
			glBindTextureUnit(nextFreeTextureUnit, m_Material->SpecularMap->Id());
		}
		else
		{
			shader.SetUniform("u_Material.hasSpecularMap", false);
		}

		DrawWithoutMaterial(instanceCount);
	}


	auto GlMesh::DrawWithoutMaterial(const GLsizei instanceCount) const -> void
	{
		if (m_Material->TwoSided)
		{
			glEnable(GL_CULL_FACE);
		}
		else
		{
			glDisable(GL_CULL_FACE);
		}

		glBindVertexArray(m_VertexArray);
		glDrawElementsInstanced(GL_TRIANGLES, m_NumIndices, GL_UNSIGNED_INT, nullptr, instanceCount);
	}
}
