#pragma once

#include "../Material.hpp"
#include "../vertex.h"
#include "../shaders/ShaderProgram.hpp"

#include <array>
#include <cstddef>
#include <vector>



namespace leopph::impl
{
	class Mesh
	{
		public:
			Mesh(std::vector<Vertex> vertices, std::vector<unsigned> indices, Material material, unsigned instanceBuffer);
			Mesh(const Mesh&) = delete;
			Mesh(Mesh&& other) noexcept;

			~Mesh();

			Mesh& operator=(const Mesh&) = delete;
			Mesh& operator=(Mesh&& other) = delete;

			bool operator==(const Mesh& other) const;

			void DrawShaded(ShaderProgram& shader, std::size_t nextFreeTextureUnit, std::size_t instanceCount) const;
			void DrawDepth(std::size_t instanceCount) const;

		private:
			enum {VERTEX, INDEX};

			unsigned m_VertexArray;
			std::array<unsigned, 2> m_Buffers;

			std::vector<Vertex> m_Vertices;
			std::vector<unsigned> m_Indices;
			Material m_Material;
	};
}
