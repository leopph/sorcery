#include "mesh.h"

#include <glad/glad.h>

namespace leopph
{
	Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned> indices, std::vector<Texture> textures)
		: vertices{ std::move(vertices) }, indices{ std::move(indices) }, textures{ std::move(textures) }
	{
		glGenVertexArrays(1, &m_VAO);
		glGenBuffers(1, &m_VBO);
		glGenBuffers(1, &m_EBO);

		glBindVertexArray(m_VAO);

		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
		glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(decltype(this->vertices)::value_type), this->vertices.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(unsigned), this->indices.data(), GL_STATIC_DRAW);

		// positions
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(decltype(this->vertices)::value_type), reinterpret_cast<void*>(offsetof(decltype(this->vertices)::value_type, position)));
		glEnableVertexAttribArray(0);

		// normals
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(decltype(this->vertices)::value_type), reinterpret_cast<void*>(offsetof(decltype(this->vertices)::value_type, normal)));
		glEnableVertexAttribArray(1);

		// textures
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(decltype(this->vertices)::value_type), reinterpret_cast<void*>(offsetof(decltype(this->vertices)::value_type, textureCoordinates)));
		glEnableVertexAttribArray(2);

		glBindVertexArray(0);
	}



	Mesh::Mesh(Mesh&& other) noexcept
	{
		this->m_VAO = other.m_VAO;
		this->m_VBO = other.m_VBO;
		this->m_EBO = other.m_EBO;

		other.m_VAO = 0;
		other.m_VBO = 0;
		other.m_EBO = 0;

		this->vertices = std::move(other.vertices);
		this->indices = std::move(other.indices);
		this->textures = std::move(other.textures);
	}



	Mesh& Mesh::operator=(Mesh&& other) noexcept
	{
		// REWORK TEXTURES WITH A COMPETENT COPY
	}



	Mesh::~Mesh()
	{
		glDeleteBuffers(1, &m_VBO);
		glDeleteBuffers(1, &m_EBO);
		glDeleteVertexArrays(1, &m_VAO);

		for (auto& texture : textures)
			glDeleteTextures(1, &texture.id);
	}



	void Mesh::Draw(const Shader& shader) const
	{
		unsigned diffuseNumber{ 0u };
		unsigned specularNumber{ 0u };

		// bind diffuse and specular maps
		for (unsigned i = 0; i < textures.size(); i++)
		{
			glActiveTexture(GL_TEXTURE0 + i);

			std::string number;
			std::string name{ textures[i].type };

			if (name == "texture_diffuse")
				number = std::to_string(diffuseNumber++);

			else if (name == "texture_specular")
				number = std::to_string(specularNumber++);

			shader.SetUniform(name + number, static_cast<int>(i));
			glBindTexture(GL_TEXTURE_2D, textures[i].id);
		}

		// draw
		glBindVertexArray(m_VAO);
		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		glActiveTexture(GL_TEXTURE0);
	}
}