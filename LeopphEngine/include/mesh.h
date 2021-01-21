#pragma once

#include "vertex.h"
#include "texture.h"
#include "shader.h"

#include <vector>

namespace leopph
{
	// CLASS TO REPRESENT A MINIMAL DRAWABLE OBJECT
	class Mesh
	{
	private:
		unsigned m_VAO;
		unsigned m_VBO;
		unsigned m_EBO;

	public:
		std::vector<Vertex> vertices;
		std::vector<unsigned> indices;
		std::vector<Texture> textures;

		Mesh(std::vector<Vertex> vertices, std::vector<unsigned> indices, std::vector<Texture> textures);
		~Mesh();

		Mesh(const Mesh& other) = delete;
		Mesh(Mesh&& other) noexcept;

		Mesh& operator=(const Mesh& other) = delete;
		Mesh& operator=(Mesh&& other) noexcept;

		void Draw(const Shader& shader) const;
	};
}