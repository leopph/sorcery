#pragma once

#include "StaticMaterial.hpp"
#include "Types.hpp"
#include "Vertex.hpp"

#include <memory>
#include <span>


namespace leopph
{
	class StaticMesh
	{
		public:
			StaticMesh(std::span<Vertex const> vertices, std::span<u32 const> indices);

			StaticMesh(StaticMesh const& other) = delete;
			StaticMesh& operator=(StaticMesh const& other) = delete;

			StaticMesh(StaticMesh&& other) = delete;
			StaticMesh& operator=(StaticMesh&& other) = delete;

			~StaticMesh();

			void draw() const;

		private:
			u32 mVao{};
			u32 mVbo{};
			u32 mIbo{};
			std::size_t mNumIndices;
			std::shared_ptr<StaticMaterial> mMaterial;
	};
}
