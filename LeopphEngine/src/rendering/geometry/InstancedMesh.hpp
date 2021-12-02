#pragma once

#include "MeshData.hpp"
#include "../shaders/ShaderProgram.hpp"

#include <array>
#include <cstddef>
#include <memory>
#include <vector>


namespace leopph::impl
{
	class InstancedMesh
	{
		public:
			InstancedMesh(MeshData& meshData, unsigned instanceBuffer);
			InstancedMesh(const InstancedMesh&) = delete;
			InstancedMesh(InstancedMesh&& other) = delete;

			~InstancedMesh();

			InstancedMesh& operator=(const InstancedMesh&) = delete;
			InstancedMesh& operator=(InstancedMesh&& other) = delete;

			bool operator==(const InstancedMesh& other) const;

			void DrawShaded(ShaderProgram& shader, std::size_t nextFreeTextureUnit, std::size_t instanceCount) const;
			void DrawDepth(std::size_t instanceCount) const;

			// Reload the InstancedMesh by rereading the data from its original MeshData source.
			void Update();


		private:
			enum {VERTEX, INDEX};

			unsigned m_VertexArray;
			std::array<unsigned, 2> m_Buffers;

			MeshData* const m_MeshDataSrc;
			std::size_t m_VertexCount;
			std::size_t m_IndexCount;
			std::shared_ptr<Material> m_Material;
	};
}
