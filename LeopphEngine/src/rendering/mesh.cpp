#include "mesh.h"

#include "../instances/InstanceHolder.hpp"
#include "../math/Matrix.hpp"
#include "../math/Vector.hpp"

#include "../util/logger.h"

#include <glad/glad.h>

#include <cstddef>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace leopph::impl
{
	Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned> indices, Material material) :
		m_Vertices{ std::move(vertices) }, m_Indices{ std::move(indices) }, m_Material{ std::move(material) },
		m_ModelBufferSize{ 1 }
	{
		glCreateBuffers(2, m_Buffers);
		glCreateVertexArrays(1, &m_VertexArray);

		glNamedBufferStorage(m_Buffers[VERTEX], m_Vertices.size() * sizeof(decltype(m_Vertices)::value_type), m_Vertices.data(), 0);
		glNamedBufferStorage(m_Buffers[INDEX], m_Indices.size() * sizeof(unsigned), m_Indices.data(), 0);

		glVertexArrayVertexBuffer(m_VertexArray, 0, m_Buffers[VERTEX], 0, sizeof(decltype(m_Vertices)::value_type));
		glVertexArrayElementBuffer(m_VertexArray, m_Buffers[INDEX]);

		glEnableVertexArrayAttrib(m_VertexArray, 0);
		glEnableVertexArrayAttrib(m_VertexArray, 1);
		glEnableVertexArrayAttrib(m_VertexArray, 2);
		glEnableVertexArrayAttrib(m_VertexArray, 3);
		glEnableVertexArrayAttrib(m_VertexArray, 4);
		glEnableVertexArrayAttrib(m_VertexArray, 5);
		glEnableVertexArrayAttrib(m_VertexArray, 6);
		glEnableVertexArrayAttrib(m_VertexArray, 7);
		glEnableVertexArrayAttrib(m_VertexArray, 8);
		glEnableVertexArrayAttrib(m_VertexArray, 9);
		glEnableVertexArrayAttrib(m_VertexArray, 10);

		glVertexArrayAttribFormat(m_VertexArray, 0, 3, GL_FLOAT, GL_FALSE, offsetof(decltype(this->m_Vertices)::value_type, position));
		glVertexArrayAttribFormat(m_VertexArray, 1, 3, GL_FLOAT, GL_FALSE, offsetof(decltype(this->m_Vertices)::value_type, normal));
		glVertexArrayAttribFormat(m_VertexArray, 2, 2, GL_FLOAT, GL_FALSE, offsetof(decltype(this->m_Vertices)::value_type, textureCoordinates));
		glVertexArrayAttribFormat(m_VertexArray, 3, 4, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribFormat(m_VertexArray, 4, 4, GL_FLOAT, GL_FALSE, sizeof(Vector4));
		glVertexArrayAttribFormat(m_VertexArray, 5, 4, GL_FLOAT, GL_FALSE, 2 * sizeof(Vector4));
		glVertexArrayAttribFormat(m_VertexArray, 6, 4, GL_FLOAT, GL_FALSE, 3 * sizeof(Vector4));
		glVertexArrayAttribFormat(m_VertexArray, 7, 4, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribFormat(m_VertexArray, 8, 4, GL_FLOAT, GL_FALSE, sizeof(Vector4));
		glVertexArrayAttribFormat(m_VertexArray, 9, 4, GL_FLOAT, GL_FALSE, 2 * sizeof(Vector4));
		glVertexArrayAttribFormat(m_VertexArray, 10, 4, GL_FLOAT, GL_FALSE, 3 * sizeof(Vector4));

		glVertexArrayAttribBinding(m_VertexArray, 0, 0);
		glVertexArrayAttribBinding(m_VertexArray, 1, 0);
		glVertexArrayAttribBinding(m_VertexArray, 2, 0);
		glVertexArrayAttribBinding(m_VertexArray, 3, 1);
		glVertexArrayAttribBinding(m_VertexArray, 4, 1);
		glVertexArrayAttribBinding(m_VertexArray, 5, 1);
		glVertexArrayAttribBinding(m_VertexArray, 6, 1);
		glVertexArrayAttribBinding(m_VertexArray, 7, 2);
		glVertexArrayAttribBinding(m_VertexArray, 8, 2);
		glVertexArrayAttribBinding(m_VertexArray, 9, 2);
		glVertexArrayAttribBinding(m_VertexArray, 10, 2);

		glVertexArrayBindingDivisor(m_VertexArray, 1, 1);
		glVertexArrayBindingDivisor(m_VertexArray, 2, 1);

		SetModelBuffer();
	}

	Mesh::Mesh(Mesh&& other) noexcept :
		m_Vertices{ std::move(other.m_Vertices) },
		m_Indices{ std::move(other.m_Indices) },
		m_Material{ std::move(other.m_Material) },
		m_VertexArray{ other.m_VertexArray },
		m_ModelBufferSize{ other.m_ModelBufferSize }
	{
		for (int i = 0; i < s_NumBuffers; ++i)
		{
			m_Buffers[i] = other.m_Buffers[i];
			other.m_Buffers[i] = 0;
		}

		other.m_VertexArray = 0;
		other.m_ModelBufferSize = 0;
	}

	Mesh::~Mesh()
	{
		glDeleteBuffers(s_NumBuffers, m_Buffers);
		glDeleteVertexArrays(1, &m_VertexArray);
	}



	bool Mesh::operator==(const Mesh& other) const
	{
		return this->m_VertexArray == other.m_VertexArray;
	}



	void Mesh::DrawShaded(const Shader& shader, const std::vector<Matrix4>& modelMatrices, const std::vector<Matrix4>& normalMatrices, std::size_t nextFreeTextureUnit) const
	{
		if (modelMatrices.size() > m_ModelBufferSize)
		{
			auto msg{ "The number of model matrices is [" + std::to_string(modelMatrices.size()) + "] while the buffer is only for [" + std::to_string(m_ModelBufferSize) + "] matrices." };
			Logger::Instance().Error(msg);
			throw std::runtime_error{ msg };
		}

		shader.SetUniform("material.ambientColor", static_cast<Vector3>(m_Material.ambientColor));
		shader.SetUniform("material.diffuseColor", static_cast<Vector3>(m_Material.diffuseColor));
		shader.SetUniform("material.specularColor", static_cast<Vector3>(m_Material.specularColor));
		shader.SetUniform("material.shininess", m_Material.shininess);

		if (m_Material.ambientMap != nullptr)
		{
			shader.SetUniform("material.hasAmbientMap", true);
			shader.SetUniform("material.ambientMap", static_cast<int>(nextFreeTextureUnit));
			glBindTextureUnit(static_cast<GLuint>(nextFreeTextureUnit), m_Material.ambientMap->id);
			++nextFreeTextureUnit;
		}
		else
		{
			shader.SetUniform("material.hasAmbientMap", false);
		}

		if (m_Material.diffuseMap != nullptr)
		{
			shader.SetUniform("material.hasDiffuseMap", true);
			shader.SetUniform("material.diffuseMap", static_cast<int>(nextFreeTextureUnit));
			glBindTextureUnit(static_cast<GLuint>(nextFreeTextureUnit), m_Material.diffuseMap->id);
			++nextFreeTextureUnit;
		}
		else
		{
			shader.SetUniform("material.hasDiffuseMap", false);
		}

		if (m_Material.specularMap != nullptr)
		{
			shader.SetUniform("material.hasSpecularMap", true);
			shader.SetUniform("material.specularMap", static_cast<int>(nextFreeTextureUnit));
			glBindTextureUnit(static_cast<GLuint>(nextFreeTextureUnit), m_Material.specularMap->id);
			++nextFreeTextureUnit;
		}
		else
		{
			shader.SetUniform("material.hasSpecularMap", false);
		}

		glNamedBufferSubData(m_Buffers[MODEL], 0, modelMatrices.size() * sizeof(std::remove_reference<decltype(modelMatrices)>::type::value_type), modelMatrices.data());
		glNamedBufferSubData(m_Buffers[NORMAL], 0, modelMatrices.size() * sizeof(std::remove_reference<decltype(normalMatrices)>::type::value_type), normalMatrices.data());

		glBindVertexArray(m_VertexArray);
		glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(m_Indices.size()), GL_UNSIGNED_INT, nullptr, static_cast<GLsizei>(modelMatrices.size()));
		glBindVertexArray(0);
	}


	void Mesh::DrawDepth(const std::vector<Matrix4>& modelMatrices) const
	{
		glNamedBufferSubData(m_Buffers[MODEL], 0, modelMatrices.size() * sizeof(std::remove_reference_t<decltype(modelMatrices)>::value_type), modelMatrices.data());
		glBindVertexArray(m_VertexArray);
		glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(m_Indices.size()), GL_UNSIGNED_INT, nullptr, static_cast<GLsizei>(modelMatrices.size()));
		glBindVertexArray(0);
	}


	void Mesh::OnReferringObjectsChanged(std::size_t newAmount) const
	{
		if (newAmount > m_ModelBufferSize)
		{
			m_ModelBufferSize *= 2;
			SetModelBuffer();
		}
		else if (newAmount * 2 < m_ModelBufferSize)
		{
			m_ModelBufferSize /= 2;
			SetModelBuffer();
		}
	}

	void Mesh::SetModelBuffer() const
	{
		glDeleteBuffers(2, &m_Buffers[MODEL]);
		glCreateBuffers(2, &m_Buffers[MODEL]);

		glNamedBufferStorage(m_Buffers[MODEL], m_ModelBufferSize * sizeof(Matrix4), nullptr, GL_DYNAMIC_STORAGE_BIT);
		glNamedBufferStorage(m_Buffers[NORMAL], m_ModelBufferSize * sizeof(Matrix4), nullptr, GL_DYNAMIC_STORAGE_BIT);

		glVertexArrayVertexBuffer(m_VertexArray, 1, m_Buffers[MODEL], 0, sizeof(Matrix4));
		glVertexArrayVertexBuffer(m_VertexArray, 2, m_Buffers[NORMAL], 0, sizeof(Matrix4));
	}
}