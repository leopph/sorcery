#pragma once

#include "MeshData.hpp"
#include "../shaders/ShaderProgram.hpp"

#include <array>
#include <cstddef>
#include <memory>


namespace leopph::impl
{
	/* GlMesh is the smallest unit of rendering.
	 * It acts as an opaque handle to OpenGL buffers ready for rendering.
	 * It also provides simple abstractions to make rendering easier.
	 * GlMesh instances only hold minimal data - copies share a single shared space in memory and keep reference counts. */
	class GlMesh
	{
		public:
			GlMesh(MeshData& meshData, unsigned instanceBuffer);

			GlMesh(const GlMesh& other);
			GlMesh& operator=(const GlMesh& other);

			GlMesh(GlMesh&& other) noexcept;
			GlMesh& operator=(GlMesh&& other) noexcept;

			~GlMesh();

			bool operator==(const GlMesh& other) const;

			void DrawShaded(ShaderProgram& shader, std::size_t nextFreeTextureUnit, std::size_t instanceCount) const;
			void DrawDepth(std::size_t instanceCount) const;

		private:
			// Decrements ref count and deletes GL resources if necessary.
			void Deinit() const;

			constexpr static std::size_t VERTEX_BUFFER{0ull};
			constexpr static std::size_t INDEX_BUFFER{1ull};


			struct SharedData
			{
				// Must not be null when in use!
				std::shared_ptr<Material> Material{nullptr};
				unsigned VertexArray{0u};
				std::array<unsigned, 2> Buffers{0u, 0u};
				std::size_t VertexCount{0};
				std::size_t IndexCount{0};
				std::size_t RefCount{1ull};
			};


			std::shared_ptr<SharedData> m_SharedData{nullptr};
	};
}
