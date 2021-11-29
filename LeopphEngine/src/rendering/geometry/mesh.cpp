#include "Mesh.hpp"

#include "AssimpModel.hpp"
#include "../../data/DataManager.hpp"
#include "../../math/Matrix.hpp"
#include "../../math/Vector.hpp"
#include "../../util/logger.h"

#include <glad/gl.h>

#include <algorithm>
#include <cstddef>
#include <string>
#include <type_traits>
#include <utility>



namespace leopph::impl
{
	Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned> indices, Material material, unsigned instanceBuffer) :
		m_Vertices{std::move(vertices)},
		m_Indices{std::move(indices)},
		m_Material{std::move(material)}
	{
		glCreateBuffers(static_cast<GLsizei>(m_Buffers.size()), m_Buffers.data());
		glCreateVertexArrays(1, &m_VertexArray);

		glNamedBufferStorage(m_Buffers[VERTEX], m_Vertices.size() * sizeof(decltype(m_Vertices)::value_type), m_Vertices.data(), 0);
		glNamedBufferStorage(m_Buffers[INDEX], m_Indices.size() * sizeof(unsigned), m_Indices.data(), 0);

		glVertexArrayVertexBuffers(m_VertexArray, 0, 2, std::array{m_Buffers[VERTEX], instanceBuffer}.data(), std::array{static_cast<GLintptr>(0), static_cast<GLintptr>(0)}.data(), std::array{static_cast<GLint>(sizeof(decltype(m_Vertices)::value_type)), static_cast<GLint>(sizeof(std::pair<Matrix4, Matrix4>) )}.data());

		glVertexArrayElementBuffer(m_VertexArray, m_Buffers[INDEX]);


		glVertexArrayAttribFormat(m_VertexArray, 0, 3, GL_FLOAT, GL_FALSE, offsetof(decltype(this->m_Vertices)::value_type, position));
		glVertexArrayAttribBinding(m_VertexArray, 0, 0);
		glEnableVertexArrayAttrib(m_VertexArray, 0);

		glVertexArrayAttribFormat(m_VertexArray, 1, 3, GL_FLOAT, GL_FALSE, offsetof(decltype(this->m_Vertices)::value_type, normal));
		glVertexArrayAttribBinding(m_VertexArray, 1, 0);
		glEnableVertexArrayAttrib(m_VertexArray, 1);

		glVertexArrayAttribFormat(m_VertexArray, 2, 2, GL_FLOAT, GL_FALSE, offsetof(decltype(this->m_Vertices)::value_type, textureCoordinates));
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

		glEnableVertexArrayAttrib(m_VertexArray, 7);
		glVertexArrayAttribFormat(m_VertexArray, 7, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(Vector4));
		glVertexArrayAttribBinding(m_VertexArray, 7, 1);

		glVertexArrayAttribFormat(m_VertexArray, 8, 4, GL_FLOAT, GL_FALSE, 5 *sizeof(Vector4));
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


	Mesh::Mesh(Mesh&& other) noexcept :
		m_Buffers{},
		m_Vertices{std::move(other.m_Vertices)},
		m_VertexArray{other.m_VertexArray},
		m_Indices{std::move(other.m_Indices)},
		m_Material{std::move(other.m_Material)}
	{
		for (auto i = 0u; i < m_Buffers.size(); ++i)
		{
			m_Buffers[i] = other.m_Buffers[i];
			other.m_Buffers[i] = 0;
		}

		other.m_VertexArray = 0;
	}


	Mesh::~Mesh()
	{
		glDeleteBuffers(static_cast<GLsizei>(m_Buffers.size()), m_Buffers.data());
		glDeleteVertexArrays(1, &m_VertexArray);
	}


	bool Mesh::operator==(const Mesh& other) const
	{
		return this->m_VertexArray == other.m_VertexArray;
	}


	void Mesh::DrawShaded(ShaderProgram& shader, std::size_t nextFreeTextureUnit, std::size_t instanceCount) const
	{
		shader.SetUniform("material.ambientColor", static_cast<Vector3>(m_Material.AmbientColor));
		shader.SetUniform("material.diffuseColor", static_cast<Vector3>(m_Material.DiffuseColor));
		shader.SetUniform("material.specularColor", static_cast<Vector3>(m_Material.SpecularColor));
		shader.SetUniform("material.shininess", m_Material.Shininess);

		if (m_Material.AmbientMap.has_value())
		{
			shader.SetUniform("material.hasAmbientMap", true);
			shader.SetUniform("material.ambientMap", static_cast<int>(nextFreeTextureUnit));
			glBindTextureUnit(static_cast<GLuint>(nextFreeTextureUnit), m_Material.AmbientMap->Id());
			++nextFreeTextureUnit;
		}
		else
		{
			shader.SetUniform("material.hasAmbientMap", false);
		}

		if (m_Material.DiffuseMap.has_value())
		{
			shader.SetUniform("material.hasDiffuseMap", true);
			shader.SetUniform("material.diffuseMap", static_cast<int>(nextFreeTextureUnit));
			glBindTextureUnit(static_cast<GLuint>(nextFreeTextureUnit), m_Material.DiffuseMap->Id());
			++nextFreeTextureUnit;
		}
		else
		{
			shader.SetUniform("material.hasDiffuseMap", false);
		}

		if (m_Material.SpecularMap.has_value())
		{
			shader.SetUniform("material.hasSpecularMap", true);
			shader.SetUniform("material.specularMap", static_cast<int>(nextFreeTextureUnit));
			glBindTextureUnit(static_cast<GLuint>(nextFreeTextureUnit), m_Material.SpecularMap->Id());
			++nextFreeTextureUnit;
		}
		else
		{
			shader.SetUniform("material.hasSpecularMap", false);
		}

		if (!m_Material.TwoSided)
		{
			glDisable(GL_CULL_FACE);
		}

		glBindVertexArray(m_VertexArray);
		glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(m_Indices.size()), GL_UNSIGNED_INT, nullptr, static_cast<GLsizei>(instanceCount));
		glBindVertexArray(0);

		if (!m_Material.TwoSided)
		{
			glEnable(GL_CULL_FACE);
		}
	}


	void Mesh::DrawDepth(std::size_t instanceCount) const
	{
		if (!m_Material.TwoSided)
		{
			glDisable(GL_CULL_FACE);
		}

		glBindVertexArray(m_VertexArray);
		glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(m_Indices.size()), GL_UNSIGNED_INT, nullptr, static_cast<GLsizei>(instanceCount));
		glBindVertexArray(0);

		if (!m_Material.TwoSided)
		{
			glEnable(GL_CULL_FACE);
		}
	}
}
