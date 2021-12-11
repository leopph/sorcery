#include "InstancedMesh.hpp"

#include "../../math/Matrix.hpp"
#include "../../math/Vector.hpp"

#include <glad/glad.h>

#include <cstddef>
#include <utility>


namespace leopph::impl
{
	InstancedMesh::InstancedMesh(MeshData& meshData, unsigned instanceBuffer) :
		m_VertexArray{},
		m_Buffers{},
		m_MeshDataSrc{&meshData},
		m_VertexCount{meshData.Vertices.size()},
		m_IndexCount{meshData.Indices.size()},
		m_Material{meshData.Material}
	{
		glCreateVertexArrays(1, &m_VertexArray);

		if (m_IndexCount > 0ull)
		{
			glCreateBuffers(static_cast<GLsizei>(m_Buffers.size()), m_Buffers.data());
			glNamedBufferStorage(m_Buffers[INDEX], meshData.Indices.size() * sizeof(unsigned), meshData.Indices.data(), 0);
			glVertexArrayElementBuffer(m_VertexArray, m_Buffers[INDEX]);
		}
		else
		{
			glCreateBuffers(1, &m_Buffers[VERTEX]);
		}

		glNamedBufferStorage(m_Buffers[VERTEX], meshData.Vertices.size() * sizeof(decltype(meshData.Vertices)::value_type), meshData.Vertices.data(), 0);
		glVertexArrayVertexBuffers(m_VertexArray, 0, 2, std::array{m_Buffers[VERTEX], instanceBuffer}.data(), std::array{static_cast<GLintptr>(0), static_cast<GLintptr>(0)}.data(), std::array{static_cast<GLint>(sizeof(decltype(meshData.Vertices)::value_type)), static_cast<GLint>(sizeof(std::pair<Matrix4, Matrix4>) )}.data());

		glVertexArrayAttribFormat(m_VertexArray, 0, 3, GL_FLOAT, GL_FALSE, offsetof(decltype(meshData.Vertices)::value_type, position));
		glVertexArrayAttribBinding(m_VertexArray, 0, 0);
		glEnableVertexArrayAttrib(m_VertexArray, 0);

		glVertexArrayAttribFormat(m_VertexArray, 1, 3, GL_FLOAT, GL_FALSE, offsetof(decltype(meshData.Vertices)::value_type, normal));
		glVertexArrayAttribBinding(m_VertexArray, 1, 0);
		glEnableVertexArrayAttrib(m_VertexArray, 1);

		glVertexArrayAttribFormat(m_VertexArray, 2, 2, GL_FLOAT, GL_FALSE, offsetof(decltype(meshData.Vertices)::value_type, textureCoordinates));
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


	InstancedMesh::~InstancedMesh()
	{
		glDeleteBuffers(static_cast<GLsizei>(m_Buffers.size()), m_Buffers.data());
		glDeleteVertexArrays(1, &m_VertexArray);
	}


	bool InstancedMesh::operator==(const InstancedMesh& other) const
	{
		return this->m_VertexArray == other.m_VertexArray;
	}


	void InstancedMesh::DrawShaded(ShaderProgram& shader, std::size_t nextFreeTextureUnit, std::size_t instanceCount) const
	{
		shader.SetUniform("u_Material.ambientColor", static_cast<Vector3>(m_Material->AmbientColor));
		shader.SetUniform("u_Material.diffuseColor", static_cast<Vector3>(m_Material->DiffuseColor));
		shader.SetUniform("u_Material.specularColor", static_cast<Vector3>(m_Material->SpecularColor));
		shader.SetUniform("u_Material.shininess", m_Material->Shininess);

		if (m_Material->AmbientMap.has_value())
		{
			shader.SetUniform("u_Material.hasAmbientMap", true);
			shader.SetUniform("u_Material.ambientMap", static_cast<int>(nextFreeTextureUnit));
			glBindTextureUnit(static_cast<GLuint>(nextFreeTextureUnit), m_Material->AmbientMap->Id());
			++nextFreeTextureUnit;
		}
		else
		{
			shader.SetUniform("u_Material.hasAmbientMap", false);
		}

		if (m_Material->DiffuseMap.has_value())
		{
			shader.SetUniform("u_Material.hasDiffuseMap", true);
			shader.SetUniform("u_Material.diffuseMap", static_cast<int>(nextFreeTextureUnit));
			glBindTextureUnit(static_cast<GLuint>(nextFreeTextureUnit), m_Material->DiffuseMap->Id());
			++nextFreeTextureUnit;
		}
		else
		{
			shader.SetUniform("u_Material.hasDiffuseMap", false);
		}

		if (m_Material->SpecularMap.has_value())
		{
			shader.SetUniform("u_Material.hasSpecularMap", true);
			shader.SetUniform("u_Material.specularMap", static_cast<int>(nextFreeTextureUnit));
			glBindTextureUnit(static_cast<GLuint>(nextFreeTextureUnit), m_Material->SpecularMap->Id());
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


	void InstancedMesh::DrawDepth(std::size_t instanceCount) const
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

	void InstancedMesh::Update()
	{
		glDeleteBuffers(static_cast<GLsizei>(m_Buffers.size()), m_Buffers.data());

		m_VertexCount = m_MeshDataSrc->Vertices.size();
		m_IndexCount = m_MeshDataSrc->Vertices.size();
		m_Material = m_MeshDataSrc->Material;

		if (m_IndexCount > 0ull)
		{
			glCreateBuffers(static_cast<GLsizei>(m_Buffers.size()), m_Buffers.data());
			glNamedBufferStorage(m_Buffers[INDEX], m_MeshDataSrc->Indices.size() * sizeof(unsigned), m_MeshDataSrc->Indices.data(), 0);
			glVertexArrayElementBuffer(m_VertexArray, m_Buffers[INDEX]);
		}
		else
		{
			glCreateBuffers(1, &m_Buffers[VERTEX]);
		}

		glNamedBufferStorage(m_Buffers[VERTEX], m_MeshDataSrc->Vertices.size() * sizeof(decltype(m_MeshDataSrc->Vertices)::value_type), m_MeshDataSrc->Vertices.data(), 0);
		glVertexArrayVertexBuffer(m_VertexArray, 0, m_Buffers[VERTEX], 0, static_cast<GLsizei>(sizeof(decltype(m_MeshDataSrc->Vertices)::value_type)));
	}
}
