#include "mesh.h"

#include "../instances/instanceholder.h"
#include "../math/matrix.h"
#include "../math/vector.h"

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

		glVertexArrayAttribFormat(m_VertexArray, 0, 3, GL_FLOAT, GL_FALSE, offsetof(decltype(this->m_Vertices)::value_type, position));
		glVertexArrayAttribFormat(m_VertexArray, 1, 3, GL_FLOAT, GL_FALSE, offsetof(decltype(this->m_Vertices)::value_type, normal));
		glVertexArrayAttribFormat(m_VertexArray, 2, 2, GL_FLOAT, GL_FALSE, offsetof(decltype(this->m_Vertices)::value_type, textureCoordinates));

		glVertexArrayAttribBinding(m_VertexArray, 0, 0);
		glVertexArrayAttribBinding(m_VertexArray, 1, 0);
		glVertexArrayAttribBinding(m_VertexArray, 2, 0);

		/*glGenVertexArrays(1, &m_VAO);
		glGenBuffers(1, &m_VBO);
		glGenBuffers(1, &m_EBO);
		glGenBuffers(1, &m_ModelBuffer);
		glGenBuffers(1, &m_NormalBuffer);

		glBindVertexArray(m_VAO);

		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
		glBufferData(GL_ARRAY_BUFFER, this->m_Vertices.size() * sizeof(decltype(this->m_Vertices)::value_type), this->m_Vertices.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->m_Indices.size() * sizeof(unsigned), this->m_Indices.data(), GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(decltype(this->m_Vertices)::value_type), reinterpret_cast<void*>(offsetof(decltype(this->m_Vertices)::value_type, position)));
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(decltype(this->m_Vertices)::value_type), reinterpret_cast<void*>(offsetof(decltype(this->m_Vertices)::value_type, normal)));
		glEnableVertexAttribArray(1);

		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(decltype(this->m_Vertices)::value_type), reinterpret_cast<void*>(offsetof(decltype(this->m_Vertices)::value_type, textureCoordinates)));
		glEnableVertexAttribArray(2);*/

		SetModelBuffer();

		//glBindVertexArray(0);

		InstanceHolder::IncMesh(m_VertexArray);
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

		//InstanceHolder::IncMesh(m_ID);
	}

	Mesh::~Mesh()
	{
		CleanUp();
	}


	leopph::impl::Mesh& Mesh::operator=(Mesh&& other) noexcept
	{
		CleanUp();

		m_VertexArray = other.m_VertexArray;
		m_ModelBufferSize = other.m_ModelBufferSize;

		for (int i = 0; i < s_NumBuffers; ++i)
		{
			m_Buffers[i] = other.m_Buffers[i];
			other.m_Buffers[i] = 0;
		}

		other.m_VertexArray = 0;
		other.m_ModelBufferSize = 0;

		m_Vertices = std::move(other.m_Vertices);
		m_Indices = std::move(m_Indices);
		m_Material = std::move(m_Material);

		//InstanceHolder::IncMesh(m_ID);

		return *this;
	}



	bool Mesh::operator==(const Mesh& other) const
	{
		return this->m_VertexArray == other.m_VertexArray;
	}



	void Mesh::Draw(const Shader& shader, const std::vector<Matrix4>& modelMatrices, const std::vector<Matrix4>& normalMatrices) const
	{
		if (modelMatrices.size() > m_ModelBufferSize)
		{
			auto msg{ "The number of model matrices is [" + std::to_string(modelMatrices.size()) + "] while the buffer is only for [" + std::to_string(m_ModelBufferSize) + "] matrices." };
			Logger::Instance().Error(msg);
			throw std::runtime_error{ msg };
		}

		shader.SetUniform("materialDiffuseColor", leopph::Vector3{ m_Material.m_DiffuseColor.red / 255.f, m_Material.m_DiffuseColor.green / 255.f, m_Material.m_DiffuseColor.blue / 255.f });
		shader.SetUniform("materialSpecularColor", leopph::Vector3{ m_Material.m_SpecularColor.red / 255.f, m_Material.m_SpecularColor.green / 255.f, m_Material.m_SpecularColor.blue / 255.f });

		std::size_t texCount{ 0 };

		if (m_Material.m_DiffuseTexture != nullptr)
		{
			glActiveTexture(GL_TEXTURE0 + static_cast<GLenum>(texCount));
			shader.SetUniform("materialHasDiffuseMap", true);
			shader.SetUniform("materialDiffuseMap", static_cast<int>(texCount));
			shader.SetUniform("materialDiffuseMapIsTransparent", m_Material.m_DiffuseTexture->isTransparent);
			glBindTexture(GL_TEXTURE_2D, m_Material.m_DiffuseTexture->id);
			texCount++;
		}
		else
		{
			shader.SetUniform("materialHasDiffuseMap", false);
		}

		if (m_Material.m_SpecularTexture != nullptr)
		{
			glActiveTexture(GL_TEXTURE0 + static_cast<GLenum>(texCount));
			shader.SetUniform("materialHasSpecularMap", true);
			shader.SetUniform("materialSpecularMap", static_cast<int>(texCount));
			shader.SetUniform("materialSpecularMapIsTransparent", m_Material.m_SpecularTexture->isTransparent);
			glBindTexture(GL_TEXTURE_2D, m_Material.m_SpecularTexture->id);
			texCount++;
		}
		else
		{
			shader.SetUniform("materialHasSpecularMap", false);
		}

		glNamedBufferSubData(m_Buffers[MODEL], 0, modelMatrices.size() * sizeof(std::remove_reference<decltype(modelMatrices)>::type::value_type), modelMatrices.data());
		glNamedBufferSubData(m_Buffers[NORMAL], 0, modelMatrices.size() * sizeof(std::remove_reference<decltype(normalMatrices)>::type::value_type), normalMatrices.data());

		glBindVertexArray(m_VertexArray);
		glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(m_Indices.size()), GL_UNSIGNED_INT, nullptr, static_cast<GLsizei>(modelMatrices.size()));
		glBindVertexArray(0);

		glActiveTexture(GL_TEXTURE0);
	}

	void Mesh::CleanUp()
	{
		InstanceHolder::DecMesh(m_VertexArray);

		if (InstanceHolder::MeshCount(m_VertexArray) == 0)
		{
			glDeleteBuffers(s_NumBuffers, m_Buffers);
			glDeleteVertexArrays(1, &m_VertexArray);
		}
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

		glEnableVertexArrayAttrib(m_VertexArray, 3);
		glEnableVertexArrayAttrib(m_VertexArray, 4);
		glEnableVertexArrayAttrib(m_VertexArray, 5);
		glEnableVertexArrayAttrib(m_VertexArray, 6);
		glEnableVertexArrayAttrib(m_VertexArray, 7);
		glEnableVertexArrayAttrib(m_VertexArray, 8);
		glEnableVertexArrayAttrib(m_VertexArray, 9);
		glEnableVertexArrayAttrib(m_VertexArray, 10);

		glVertexArrayAttribFormat(m_VertexArray, 3, 4, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribFormat(m_VertexArray, 4, 4, GL_FLOAT, GL_FALSE, sizeof(Vector4));
		glVertexArrayAttribFormat(m_VertexArray, 5, 4, GL_FLOAT, GL_FALSE, 2 * sizeof(Vector4));
		glVertexArrayAttribFormat(m_VertexArray, 6, 4, GL_FLOAT, GL_FALSE, 3 * sizeof(Vector4));
		glVertexArrayAttribFormat(m_VertexArray, 7, 4, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribFormat(m_VertexArray, 8, 4, GL_FLOAT, GL_FALSE, sizeof(Vector4));
		glVertexArrayAttribFormat(m_VertexArray, 9, 4, GL_FLOAT, GL_FALSE, 2 * sizeof(Vector4));
		glVertexArrayAttribFormat(m_VertexArray, 10, 4, GL_FLOAT, GL_FALSE, 3 * sizeof(Vector4));

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

		/*glBindVertexArray(m_VertexArray);

		glBindBuffer(GL_ARRAY_BUFFER, m_ModelBuffer);
		glBufferData(GL_ARRAY_BUFFER, m_ModelBufferSize * sizeof(Matrix4), nullptr, GL_STATIC_DRAW);
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Matrix4), reinterpret_cast<void*>(0));
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Matrix4), reinterpret_cast<void*>(sizeof(Vector4)));
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(Matrix4), reinterpret_cast<void*>(2 * sizeof(Vector4)));
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Matrix4), reinterpret_cast<void*>(3 * sizeof(Vector4)));
		glEnableVertexAttribArray(3);
		glEnableVertexAttribArray(4);
		glEnableVertexAttribArray(5);
		glEnableVertexAttribArray(6);
		glVertexAttribDivisor(3, 1);
		glVertexAttribDivisor(4, 1);
		glVertexAttribDivisor(5, 1);
		glVertexAttribDivisor(6, 1);

		glBindBuffer(GL_ARRAY_BUFFER, m_NormalBuffer);
		glBufferData(GL_ARRAY_BUFFER, m_ModelBufferSize * sizeof(Matrix4), nullptr, GL_STATIC_DRAW);
		glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(Matrix4), reinterpret_cast<void*>(0));
		glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(Matrix4), reinterpret_cast<void*>(sizeof(Vector4)));
		glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, sizeof(Matrix4), reinterpret_cast<void*>(2 * sizeof(Vector4)));
		glVertexAttribPointer(10, 4, GL_FLOAT, GL_FALSE, sizeof(Matrix4), reinterpret_cast<void*>(3 * sizeof(Vector4)));
		glEnableVertexAttribArray(7);
		glEnableVertexAttribArray(8);
		glEnableVertexAttribArray(9);
		glEnableVertexAttribArray(10);
		glVertexAttribDivisor(7, 1);
		glVertexAttribDivisor(8, 1);
		glVertexAttribDivisor(9, 1);
		glVertexAttribDivisor(10, 1);

		glBindVertexArray(0);*/
	}

}