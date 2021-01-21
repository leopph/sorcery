#include "mesh.h"

#include <glad/glad.h>

namespace leopph
{
	// INIT REF COUNTER
	std::unordered_map<unsigned, size_t> Mesh::s_Instances{};


	// LOAD MESH FROM MESH DATA
	Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned> indices, std::vector<Texture> textures)
		: m_Vertices{ std::move(vertices) }, m_Indices{ std::move(indices) }, m_Textures{ std::move(textures) }
	{
		glGenVertexArrays(1, &m_VAO);
		glGenBuffers(1, &m_VBO);
		glGenBuffers(1, &m_EBO);

		glBindVertexArray(m_VAO);

		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
		glBufferData(GL_ARRAY_BUFFER, this->m_Vertices.size() * sizeof(decltype(this->m_Vertices)::value_type), this->m_Vertices.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->m_Indices.size() * sizeof(unsigned), this->m_Indices.data(), GL_STATIC_DRAW);

		// positions
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(decltype(this->m_Vertices)::value_type), reinterpret_cast<void*>(offsetof(decltype(this->m_Vertices)::value_type, position)));
		glEnableVertexAttribArray(0);

		// normals
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(decltype(this->m_Vertices)::value_type), reinterpret_cast<void*>(offsetof(decltype(this->m_Vertices)::value_type, normal)));
		glEnableVertexAttribArray(1);

		// textures
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(decltype(this->m_Vertices)::value_type), reinterpret_cast<void*>(offsetof(decltype(this->m_Vertices)::value_type, textureCoordinates)));
		glEnableVertexAttribArray(2);

		glBindVertexArray(0);

		s_Instances.try_emplace(m_VAO, 0);
		s_Instances[m_VAO]++;
	}



	Mesh::~Mesh()
	{
		s_Instances[m_VAO]--;

		if (s_Instances[m_VAO] == 0)
		{
			glDeleteBuffers(1, &m_VBO);
			glDeleteBuffers(1, &m_EBO);
			glDeleteVertexArrays(1, &m_VAO);
		}
	}



	Mesh::Mesh(const Mesh& other)
		: m_VAO{ other.m_VAO }, m_VBO{ other.m_VBO }, m_EBO{ other.m_EBO },
		m_Vertices{ other.m_Vertices }, m_Indices{ other.m_Indices }, m_Textures{ other.m_Textures }
	{
		s_Instances[m_VAO]++;
	}



	Mesh::Mesh(Mesh&& other) noexcept
		: m_VAO{ other.m_VAO }, m_VBO{ other.m_VBO }, m_EBO{ other.m_EBO },
		m_Vertices{ std::move(other.m_Vertices) }, m_Indices{ std::move(other.m_Indices) },
		m_Textures{ std::move(other.m_Textures) }
	{
		other.m_VAO = 0;
		other.m_VBO = 0;
		other.m_EBO = 0;

		s_Instances.try_emplace(0, 0);
		s_Instances[0]++;
	}



	Mesh& Mesh::operator=(const Mesh& other)
	{
		if (*this == other)
			return *this;

		s_Instances[m_VAO]--;

		if (s_Instances[m_VAO] == 0)
		{
			glDeleteBuffers(1, &m_VBO);
			glDeleteBuffers(1, &m_EBO);
			glDeleteVertexArrays(1, &m_VAO);
		}

		this->m_VAO = other.m_VAO;
		this->m_VBO = other.m_VBO;
		this->m_EBO = other.m_EBO;

		this->m_Vertices = other.m_Vertices;
		this->m_Indices = other.m_Indices;
		this->m_Textures = other.m_Textures;

		s_Instances[m_VAO]++;

		return *this;
	}



	Mesh& Mesh::operator=(Mesh&& other) noexcept
	{
		if (*this == other)
			return *this;

		s_Instances[m_VAO]--;

		if (s_Instances[m_VAO] == 0)
		{
			glDeleteBuffers(1, &m_VBO);
			glDeleteBuffers(1, &m_EBO);
			glDeleteVertexArrays(1, &m_VAO);
		}

		this->m_VAO = other.m_VAO;
		this->m_VBO = other.m_VBO;
		this->m_EBO = other.m_EBO;

		this->m_Vertices = std::move(other.m_Vertices);
		this->m_Indices = std::move(other.m_Indices);
		this->m_Textures = std::move(other.m_Textures);

		other.m_VAO = 0;
		other.m_VBO = 0;
		other.m_EBO = 0;

		s_Instances.try_emplace(0, 0);
		s_Instances[0]++;

		return *this;
	}



	bool Mesh::operator==(const Mesh& other) const
	{
		return this->m_VAO == other.m_VAO;
	}



	void Mesh::Draw(const Shader& shader) const
	{
		unsigned diffuseNumber{ 0u };
		unsigned specularNumber{ 0u };

		// bind diffuse and specular maps
		for (unsigned i = 0; i < m_Textures.size(); i++)
		{
			glActiveTexture(GL_TEXTURE0 + i);

			std::string number;
			std::string name{ "texture_" };

			switch (m_Textures[i].Type())
			{
			case Texture::TextureType::DIFFUSE:
				number = std::to_string(diffuseNumber++);
				name += "diffuse";
				break;

			case Texture::TextureType::SPECULAR:
				number = std::to_string(specularNumber++);
				name += "specular";
				break;
			}

			shader.SetUniform(name + number, static_cast<int>(i));
			glBindTexture(GL_TEXTURE_2D, m_Textures[i].ID());
		}

		// draw
		glBindVertexArray(m_VAO);
		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_Indices.size()), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		glActiveTexture(GL_TEXTURE0);
	}
}