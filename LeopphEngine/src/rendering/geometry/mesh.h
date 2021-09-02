#pragma once

#include "../material.h"
#include "../Shader.hpp"
#include "../vertex.h"

#include <cstddef>
#include <vector>

namespace leopph::impl
{
	class Mesh
	{
	public:
		Mesh(std::vector<Vertex> vertices, std::vector<unsigned> indices, Material material);
		Mesh(const Mesh&) = delete;
		Mesh(Mesh&& other) noexcept;

		~Mesh();

		Mesh& operator=(const Mesh&) = delete;
		Mesh& operator=(Mesh&& other) = delete;

		bool operator==(const Mesh& other) const;

		void DrawShaded(const Shader& shader, const std::vector<Matrix4>& modelMatrices, const std::vector<Matrix4>& normalMatrices, std::size_t nextFreeTextureUnit) const;
		void DrawDepth(const std::vector<Matrix4>& modelMatrices) const;

		void OnReferringObjectsChanged(std::size_t newAmount) const;

	private:
		void SetModelBuffer() const;

		enum { VERTEX, INDEX, MODEL, NORMAL };
		static constexpr std::size_t s_NumBuffers{ 4 };

		unsigned m_VertexArray;
		mutable unsigned m_Buffers[s_NumBuffers];
		mutable std::size_t m_ModelBufferSize;

		std::vector<Vertex> m_Vertices;
		std::vector<unsigned> m_Indices;
		Material m_Material;
	};
}