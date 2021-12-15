#include "GlMesh.hpp"

#include "../../math/Matrix.hpp"
#include "../../math/Vector.hpp"

#include <glad/glad.h>

#include <cstddef>
#include <utility>


namespace leopph::impl
{
	GlMesh::GlMesh(MeshData& meshData, const unsigned instanceBuffer) :
		m_Material{meshData.Material},
		m_RefCount{std::make_shared<std::size_t>(1ull)},
		m_VertexCount{meshData.Vertices.size()},
		m_IndexCount{meshData.Indices.size()}
	{
		glCreateVertexArrays(1, &m_VertexArray);

		if (m_IndexCount > 0ull)
		{
			glCreateBuffers(static_cast<GLsizei>(m_Buffers.size()), m_Buffers.data());
			glNamedBufferStorage(m_Buffers[INDEX_BUFFER], meshData.Indices.size() * sizeof(unsigned), meshData.Indices.data(), 0);
			glVertexArrayElementBuffer(m_VertexArray, m_Buffers[INDEX_BUFFER]);
		}
		else
		{
			glCreateBuffers(1, &m_Buffers[VERTEX_BUFFER]);
		}

		glNamedBufferStorage(m_Buffers[VERTEX_BUFFER], meshData.Vertices.size() * sizeof(decltype(meshData.Vertices)::value_type), meshData.Vertices.data(), 0);
		glVertexArrayVertexBuffers(m_VertexArray, 0, 2, std::array{m_Buffers[VERTEX_BUFFER], instanceBuffer}.data(), std::array{static_cast<GLintptr>(0), static_cast<GLintptr>(0)}.data(), std::array{static_cast<GLint>(sizeof(decltype(meshData.Vertices)::value_type)), static_cast<GLint>(sizeof(std::pair<Matrix4, Matrix4>))}.data());

		glVertexArrayAttribFormat(m_VertexArray, 0, 3, GL_FLOAT, GL_FALSE, offsetof(decltype(meshData.Vertices)::value_type, Position));
		glVertexArrayAttribBinding(m_VertexArray, 0, 0);
		glEnableVertexArrayAttrib(m_VertexArray, 0);

		glVertexArrayAttribFormat(m_VertexArray, 1, 3, GL_FLOAT, GL_FALSE, offsetof(decltype(meshData.Vertices)::value_type, Normal));
		glVertexArrayAttribBinding(m_VertexArray, 1, 0);
		glEnableVertexArrayAttrib(m_VertexArray, 1);

		glVertexArrayAttribFormat(m_VertexArray, 2, 2, GL_FLOAT, GL_FALSE, offsetof(decltype(meshData.Vertices)::value_type, TexCoord));
		glVertexArrayAttribBinding(m_VertexArray, 2, 0);
		glEnableVertexArrayAttrib(m_VertexArray, 2);

		glVertexArrayAttribFormat(m_VertexArray, 3, 4, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribBinding(m_VertexArray, 3, 1);
		glEnableVertexArrayAttrib(m_VertexArray, 3);

		glVertexArrayAttribFormat(m_VertexArray, 4, 4, GL_FLOAT, GL_FALSE, sizeof(Vector4));
		glVertexArrayAttribBinding(m_VertexArray, 4, 1);
		glEnableVertexArrayAttrib(m_VertexArray, 4);

		glVertexArrayAttribFormat(m_VertexArray, 5, 4, GL_FLOAT, GL_FALSE, 2 * sizeof(Vector4));
		glVertexArrayAttribBinding(m_VertexArray, 5, 1);
		glEnableVertexArrayAttrib(m_VertexArray, 5);

		glVertexArrayAttribFormat(m_VertexArray, 6, 4, GL_FLOAT, GL_FALSE, 3 * sizeof(Vector4));
		glVertexArrayAttribBinding(m_VertexArray, 6, 1);
		glEnableVertexArrayAttrib(m_VertexArray, 6);

		glVertexArrayAttribFormat(m_VertexArray, 7, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(Vector4));
		glVertexArrayAttribBinding(m_VertexArray, 7, 1);
		glEnableVertexArrayAttrib(m_VertexArray, 7);

		glVertexArrayAttribFormat(m_VertexArray, 8, 4, GL_FLOAT, GL_FALSE, 5 * sizeof(Vector4));
		glVertexArrayAttribBinding(m_VertexArray, 8, 1);
		glEnableVertexArrayAttrib(m_VertexArray, 8);

		glVertexArrayAttribFormat(m_VertexArray, 9, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(Vector4));
		glVertexArrayAttribBinding(m_VertexArray, 9, 1);
		glEnableVertexArrayAttrib(m_VertexArray, 9);

		glVertexArrayAttribFormat(m_VertexArray, 10, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(Vector4));
		glVertexArrayAttribBinding(m_VertexArray, 10, 1);
		glEnableVertexArrayAttrib(m_VertexArray, 10);

		glVertexArrayBindingDivisor(m_VertexArray, 1, 1);
	}

	GlMesh::GlMesh(const GlMesh& other) :
		m_Material{other.m_Material},
		m_RefCount{other.m_RefCount},
		m_VertexArray{other.m_VertexArray},
		m_Buffers{other.m_Buffers},
		m_VertexCount{other.m_VertexCount},
		m_IndexCount{other.m_IndexCount}
	{
		++*m_RefCount;
	}

	GlMesh& GlMesh::operator=(const GlMesh& other)
	{
		if (this == &other)
		{
			return *this;
		}

		Deinit();
		m_Material = other.m_Material;
		m_RefCount = other.m_RefCount;
		m_VertexArray = other.m_VertexArray;
		m_Buffers = other.m_Buffers;
		m_VertexCount = other.m_VertexCount;
		m_IndexCount = other.m_IndexCount;
		++*m_RefCount;
		return *this;
	}

	GlMesh::GlMesh(GlMesh&& other) noexcept :
		GlMesh{other}
	{}

	GlMesh& GlMesh::operator=(GlMesh&& other) noexcept
	{
		return *this = other;
	}

	GlMesh::~GlMesh()
	{
		Deinit();
	}


	bool GlMesh::operator==(const GlMesh& other) const
	{
		return this->m_VertexArray == other.m_VertexArray;
	}


	void GlMesh::DrawShaded(ShaderProgram& shader, std::size_t nextFreeTextureUnit, const std::size_t instanceCount) const
	{
		shader.SetUniform("u_Material.ambientColor", static_cast<Vector3>(m_Material->AmbientColor));
		shader.SetUniform("u_Material.diffuseColor", static_cast<Vector3>(m_Material->DiffuseColor));
		shader.SetUniform("u_Material.specularColor", static_cast<Vector3>(m_Material->SpecularColor));
		shader.SetUniform("u_Material.shininess", m_Material->Shininess);

		if (m_Material->AmbientMap != nullptr)
		{
			shader.SetUniform("u_Material.hasAmbientMap", true);
			shader.SetUniform("u_Material.ambientMap", static_cast<int>(nextFreeTextureUnit));
			glBindTextureUnit(static_cast<GLuint>(nextFreeTextureUnit), m_Material->AmbientMap->Id);
			++nextFreeTextureUnit;
		}
		else
		{
			shader.SetUniform("u_Material.hasAmbientMap", false);
		}

		if (m_Material->DiffuseMap != nullptr)
		{
			shader.SetUniform("u_Material.hasDiffuseMap", true);
			shader.SetUniform("u_Material.diffuseMap", static_cast<int>(nextFreeTextureUnit));
			glBindTextureUnit(static_cast<GLuint>(nextFreeTextureUnit), m_Material->DiffuseMap->Id);
			++nextFreeTextureUnit;
		}
		else
		{
			shader.SetUniform("u_Material.hasDiffuseMap", false);
		}

		if (m_Material->SpecularMap != nullptr)
		{
			shader.SetUniform("u_Material.hasSpecularMap", true);
			shader.SetUniform("u_Material.specularMap", static_cast<int>(nextFreeTextureUnit));
			glBindTextureUnit(static_cast<GLuint>(nextFreeTextureUnit), m_Material->SpecularMap->Id);
			++nextFreeTextureUnit;
		}
		else
		{
			shader.SetUniform("u_Material.hasSpecularMap", false);
		}

		if (!m_Material->TwoSided)
		{
			glDisable(GL_CULL_FACE);
		}

		glBindVertexArray(m_VertexArray);
		if (m_IndexCount > 0ull)
		{
			glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(m_IndexCount), GL_UNSIGNED_INT, nullptr, static_cast<GLsizei>(instanceCount));
		}
		else
		{
			glDrawArraysInstanced(GL_TRIANGLES, 0, static_cast<GLsizei>(m_VertexCount), static_cast<GLsizei>(instanceCount));
		}
		glBindVertexArray(0);

		if (!m_Material->TwoSided)
		{
			glEnable(GL_CULL_FACE);
		}
	}


	void GlMesh::DrawDepth(const std::size_t instanceCount) const
	{
		if (!m_Material->TwoSided)
		{
			glDisable(GL_CULL_FACE);
		}

		glBindVertexArray(m_VertexArray);
		if (m_IndexCount > 0ull)
		{
			glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(m_IndexCount), GL_UNSIGNED_INT, nullptr, static_cast<GLsizei>(instanceCount));
		}
		else
		{
			glDrawArraysInstanced(GL_TRIANGLES, 0, static_cast<GLsizei>(m_VertexCount), static_cast<GLsizei>(instanceCount));
		}
		glBindVertexArray(0);

		if (!m_Material->TwoSided)
		{
			glEnable(GL_CULL_FACE);
		}
	}

	void GlMesh::Deinit() const
	{
		--*m_RefCount;

		if (*m_RefCount == 0ull)
		{
			glDeleteBuffers(static_cast<GLsizei>(m_Buffers.size()), m_Buffers.data());
			glDeleteVertexArrays(1, &m_VertexArray);
		}
	}
}
