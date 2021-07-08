#pragma once

#include "vertex.h"
#include "texture.h"
#include "shader.h"
#include "material.h"

#include <cstddef>
#include <vector>

namespace leopph::impl
{
	class Mesh
	{
	public:
		Mesh(std::vector<Vertex> vertices, std::vector<unsigned> indices, Material material = Material{});
		Mesh(const Mesh&) = delete;
		Mesh(Mesh&& other) noexcept;
		~Mesh();

		Mesh& operator=(const Mesh&) = delete;
		Mesh& operator=(Mesh&& other) noexcept;

		bool operator==(const Mesh& other) const;

		void Draw(const Shader& shader, const std::vector<Matrix4>& modelMatrices, const std::vector<Matrix4>& normalMatrices) const;

		void OnReferringObjectsChanged(std::size_t newAmount) const;

	private:
		void CleanUp();
		void SetModelBuffer() const;

		unsigned m_VAO;
		unsigned m_VBO;
		unsigned m_EBO;
		unsigned m_ModelBuffer;
		unsigned m_NormalBuffer;
		mutable std::size_t m_ModelBufferSize;

		const unsigned& m_ID{ m_VAO };

		std::vector<Vertex> m_Vertices;
		std::vector<unsigned> m_Indices;
		Material m_Material;
	};
}