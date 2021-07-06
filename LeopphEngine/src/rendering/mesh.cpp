#include "mesh.h"

#include "../instances/instanceholder.h"

#include <glad/glad.h>

#include <cstddef>

namespace leopph::impl
{
	Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned> indices, Material material)
		: m_Vertices{ std::move(vertices) }, m_Indices{ std::move(indices) }, m_Material{ std::move(material) }
	{
		glGenVertexArrays(1, &m_VAO);
		glGenBuffers(1, &m_VBO);
		glGenBuffers(1, &m_EBO);

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
		glEnableVertexAttribArray(2);

		glBindVertexArray(0);

		InstanceHolder::AddMesh(m_ID);
	}

	Mesh::Mesh(Mesh&& other) noexcept :
		m_Vertices{ std::move(other.m_Vertices) },
		m_Indices{ std::move(other.m_Indices) },
		m_Material{ std::move(other.m_Material) },
		m_VAO{ other.m_VAO },
		m_VBO{ other.m_VBO },
		m_EBO{ other.m_EBO }
	{
		other.m_VAO = 0;
		other.m_VBO = 0;
		other.m_EBO = 0;

		InstanceHolder::AddMesh(m_ID);
	}

	Mesh::~Mesh()
	{
		CleanUp();
	}


	leopph::impl::Mesh& Mesh::operator=(Mesh&& other) noexcept
	{
		CleanUp();

		m_VAO = other.m_VAO;
		m_VBO = other.m_VBO;
		m_EBO = other.m_EBO;

		other.m_VAO = 0;
		other.m_VBO = 0;
		other.m_EBO = 0;

		m_Vertices = std::move(other.m_Vertices);
		m_Indices = std::move(m_Indices);
		m_Material = std::move(m_Material);

		InstanceHolder::AddMesh(m_ID);

		return *this;
	}



	bool Mesh::operator==(const Mesh& other) const
	{
		return this->m_VAO == other.m_VAO;
	}



	void Mesh::Draw(const Shader& shader) const
	{
		shader.SetUniform("materialDiffuseColor", leopph::Vector3{ m_Material.m_DiffuseColor.red / 255.f, m_Material.m_DiffuseColor.green / 255.f, m_Material.m_DiffuseColor.blue / 255.f });
		shader.SetUniform("materialSpecularColor", leopph::Vector3{ m_Material.m_SpecularColor.red / 255.f, m_Material.m_SpecularColor.green / 255.f, m_Material.m_SpecularColor.blue / 255.f });

		std::size_t texCount{ 0 };

		if (m_Material.m_DiffuseTexture != nullptr)
		{
			glActiveTexture(GL_TEXTURE0 + static_cast<GLenum>(texCount));
			shader.SetUniform("materialHasDiffuseMap", true);
			shader.SetUniform("materialDiffuseMap", static_cast<int>(texCount));
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
			glBindTexture(GL_TEXTURE_2D, m_Material.m_SpecularTexture->id);
			texCount++;
		}
		else
		{
			shader.SetUniform("materialHasSpecularMap", false);
		}

		glBindVertexArray(m_VAO);
		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_Indices.size()), GL_UNSIGNED_INT, nullptr);
		glBindVertexArray(0);

		glActiveTexture(GL_TEXTURE0);
	}

	void Mesh::CleanUp()
	{
		InstanceHolder::RemoveMesh(m_ID);

		if (InstanceHolder::MeshCount(m_ID) == 0)
		{
			glDeleteBuffers(1, &m_VBO);
			glDeleteBuffers(1, &m_EBO);
			glDeleteVertexArrays(1, &m_VAO);
		}
	}

}